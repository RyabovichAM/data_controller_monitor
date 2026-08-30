#ifndef TIMESCALEDB_DATA_STORAGE_H
#define TIMESCALEDB_DATA_STORAGE_H

#include <mutex>

#include <pqxx/pqxx>

#include "data_storage_interface.h"
#include "timescaledb_data_storage_domain.h"

namespace data_storage {

// One hypertable, sensor_data, holds every collector's samples — that is the
// idiomatic shape for TimescaleDB, unlike FileDataStorage's directory per
// collector. Each instance still belongs to one collector (StorageRegistry's
// contract), but the table and the schema bootstrap are shared by construction:
// CREATE TABLE IF NOT EXISTS and create_hypertable(if_not_exists => true) are
// safe to run from every instance that happens to be first for its collector.
//
// One connection, one mutex — the same trade-off PostgresConfigRepository
// makes in config-service: DataSave (the Kafka thread) and DataLoad (a gRPC
// thread browsing history) cannot run at the same instant, only one after the
// other. Config traffic and now this are both low-frequency enough that a
// connection pool would be solving a problem this project does not have yet.
class TimescaleDbDataStorage : public DataStorageInterface {
public:
    explicit TimescaleDbDataStorage(const Settings& settings);

    void SetErrorHandler(ErrorHandler handler) override;
    void DataSave(const std::string& json) override;
    void DataLoad(TimePoint from, TimePoint to, const DataSink& sink) override;
    bool Open() override;
    bool IsOpen() const override;
    void Close() override;

private:
    TimescaleDbSettings settings_;
    ErrorHandler error_handler_{nullptr};
    SampleThinner thinner_;

    std::mutex mutex_;
    std::unique_ptr<pqxx::connection> connection_;

    void EnsureSchema();
    void Reconnect();
    void ReportError(const std::string& message) const;
};

}   //data_storage

#endif // TIMESCALEDB_DATA_STORAGE_H
