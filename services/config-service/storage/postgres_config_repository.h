#ifndef POSTGRES_CONFIG_REPOSITORY_H
#define POSTGRES_CONFIG_REPOSITORY_H

#include <mutex>
#include <string>
#include <utility>

#include <pqxx/pqxx>

#include "storage/config_repository.h"

namespace config {

// A config is stored as JSONB, not as a set of columns: its schema is the
// .proto contract, so columns would have to be rewritten on every change to
// it, and a config is always read and written whole. The version is a column
// of its own — it has to be locked and compared without parsing the JSON.
class PostgresConfigRepository : public ConfigRepository {
public:
    // Connects and creates the table if it is missing. Throws
    // pqxx::broken_connection while the database is unreachable.
    explicit PostgresConfigRepository(const std::string& connection_string);

    std::optional<CollectorConfig> Get(const std::string& collector_id) override;
    std::vector<CollectorConfig> List() override;
    SaveResult Save(const CollectorConfig& config, int64_t expected_version) override;
    bool Delete(const std::string& collector_id) override;

private:
    // pqxx::connection is not thread-safe and gRPC serves calls from a pool of
    // threads. One connection under a mutex is enough for config traffic.
    std::mutex mutex_;
    std::string connection_string_;
    pqxx::connection connection_;

    void EnsureSchema();

    // Runs the operation in a transaction and commits it. A connection dropped
    // by a restarted database is reopened once and the operation retried, so a
    // postgres restart does not turn the service into a permanently failing one.
    template <typename Operation>
    auto InTransaction(Operation&& operation)
        -> decltype(operation(std::declval<pqxx::work&>()));
};

}   //config

#endif // POSTGRES_CONFIG_REPOSITORY_H
