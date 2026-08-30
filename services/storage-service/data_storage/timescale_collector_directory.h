#ifndef TIMESCALE_COLLECTOR_DIRECTORY_H
#define TIMESCALE_COLLECTOR_DIRECTORY_H

#include <memory>
#include <mutex>
#include <string>

#include <pqxx/pqxx>

#include "collector_directory.h"

namespace data_storage {

// Answers "who has anything stored" against the same sensor_data hypertable
// TimescaleDbDataStorage writes into. One connection for the lifetime of the
// service: unlike the per-collector storages, this is asked on every
// GET /api/collectors, and opening a fresh connection per request would be a
// needless round trip on the hot path of a page load.
class TimescaleCollectorDirectory : public CollectorDirectory {
public:
    explicit TimescaleCollectorDirectory(std::string connection_string);

    bool HasCollector(const std::string& collector_id) const override;
    std::vector<std::string> KnownCollectors() const override;

private:
    std::string connection_string_;

    mutable std::mutex mutex_;
    mutable std::unique_ptr<pqxx::connection> connection_;

    // const because both public methods are; the connection is a cache, not
    // part of the logical state of "what is in the database".
    pqxx::connection& Connected() const;
};

}   //data_storage

#endif // TIMESCALE_COLLECTOR_DIRECTORY_H
