#ifndef POSTGRES_SCHEME_REPOSITORY_H
#define POSTGRES_SCHEME_REPOSITORY_H

#include <mutex>
#include <string>
#include <utility>

#include <pqxx/pqxx>

#include "storage/scheme_repository.h"

namespace config {

// Schemes live in the same database as the configs, in the same shape and for
// the same reasons: the whole scheme as JSONB, because its schema is the
// .proto contract and it is always read and written whole; the version as a
// column of its own, because it has to be locked and compared without parsing
// the JSON.
//
// The summary columns — title and collector_id — are duplicated out of the
// JSON so that ListSchemes reads a narrow row per scheme instead of parsing
// every shape of every one of them.
class PostgresSchemeRepository : public SchemeRepository {
public:
    // Throws pqxx::broken_connection while the database is unreachable.
    explicit PostgresSchemeRepository(const std::string& connection_string);

    std::optional<Scheme> Get(const std::string& scheme_id) override;
    std::vector<SchemeSummary> List() override;
    SaveResult Save(const Scheme& scheme, int64_t expected_version) override;
    bool Delete(const std::string& scheme_id) override;

private:
    // pqxx::connection is not thread-safe and gRPC serves calls from a pool of
    // threads. One connection under a mutex is enough for scheme traffic.
    std::mutex mutex_;
    std::string connection_string_;
    pqxx::connection connection_;

    void EnsureSchema();

    // Runs the operation in a transaction and commits it. A connection dropped
    // by a restarted database is reopened once and the operation retried.
    template <typename Operation>
    auto InTransaction(Operation&& operation)
        -> decltype(operation(std::declval<pqxx::work&>()));
};

}   //config

#endif // POSTGRES_SCHEME_REPOSITORY_H
