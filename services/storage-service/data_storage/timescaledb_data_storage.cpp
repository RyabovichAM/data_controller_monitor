#include "timescaledb_data_storage.h"

#include <exception>
#include <utility>

namespace data_storage {

namespace {

constexpr const char* kCreateTable =
    "CREATE TABLE IF NOT EXISTS sensor_data ("
    "    collector_id TEXT        NOT NULL,"
    "    ts           TIMESTAMPTZ NOT NULL,"
    "    payload      TEXT        NOT NULL)";

// if_not_exists makes this safe to call from every collector's storage
// instance, not just the first one to reach it — TimescaleDB itself
// serialises the race and the losers get a no-op instead of an error.
constexpr const char* kCreateHypertable =
    "SELECT create_hypertable('sensor_data', 'ts', if_not_exists => true)";

constexpr const char* kCreateIndex =
    "CREATE INDEX IF NOT EXISTS sensor_data_collector_ts_idx "
    "    ON sensor_data (collector_id, ts DESC)";

}   //namespace

TimescaleDbDataStorage::TimescaleDbDataStorage(const Settings& settings)
    : settings_{GetTimescaleDbSettings(settings)}
    , thinner_{settings_.survey_period} {
    // No connection here: a read-only lookup (StorageRegistry::FindCollector)
    // builds one of these for every collector it has ever heard of, and most
    // of them will answer a single DataLoad and nothing else. Connecting is
    // deferred to first use.
}

void TimescaleDbDataStorage::SetErrorHandler(ErrorHandler handler) {
    error_handler_ = handler;
}

void TimescaleDbDataStorage::DataSave(const std::string& json) {
    const TimePoint now = std::chrono::system_clock::now();
    if (!thinner_.ShouldKeep(now)) {
        return;
    }

    std::lock_guard<std::mutex> lock{mutex_};

    try {
        if (!connection_) {
            Reconnect();
        }

        pqxx::work tx{*connection_};
        tx.exec_params(
            "INSERT INTO sensor_data (collector_id, ts, payload) "
            "VALUES ($1, to_timestamp($2), $3)",
            settings_.collector_id, ToEpochSeconds(now), json);
        tx.commit();
    } catch (const std::exception& error) {
        // A connection that just dropped is worth one more try next time
        // rather than wedging this collector's writes for good.
        connection_.reset();
        ReportError(std::string{"save: "} + error.what());
    }
}

void TimescaleDbDataStorage::DataLoad(TimePoint from, TimePoint to, const DataSink& sink) {
    if (from >= to || !sink) {
        return;
    }

    std::lock_guard<std::mutex> lock{mutex_};

    try {
        if (!connection_) {
            Reconnect();
        }

        pqxx::work tx{*connection_};

        // stream() takes no parameters of its own (unlike exec()), so the
        // values are quoted into the query text by hand. The epoch seconds are
        // pure digits/./- and cannot break out of the literal; the collector id
        // goes through quote() because it is operator-supplied text.
        const std::string query =
            "SELECT EXTRACT(EPOCH FROM ts), payload FROM sensor_data"
            " WHERE collector_id = " +
            tx.quote(settings_.collector_id) + " AND ts >= to_timestamp(" +
            std::to_string(ToEpochSeconds(from)) + ") AND ts < to_timestamp(" +
            std::to_string(ToEpochSeconds(to)) +
            ") ORDER BY ts ASC";

        // A real server-side stream: rows arrive one at a time rather than
        // filling a result set in memory, which is the same reason DataLoad
        // takes a sink instead of returning a container.
        for (auto [epoch_seconds, payload] : tx.stream<double, std::string>(query)) {
            if (!sink(DataPoint{FromEpochSeconds(epoch_seconds), payload})) {
                break;   // the destructor of the stream ends the query cleanly
            }
        }

        tx.commit();
    } catch (const std::exception& error) {
        connection_.reset();
        ReportError(std::string{"load: "} + error.what());
    }
}

bool TimescaleDbDataStorage::Open() {
    std::lock_guard<std::mutex> lock{mutex_};

    try {
        Reconnect();
        return true;
    } catch (const std::exception& error) {
        ReportError(std::string{"open: "} + error.what());
        return false;
    }
}

bool TimescaleDbDataStorage::IsOpen() const {
    return connection_ && connection_->is_open();
}

void TimescaleDbDataStorage::Close() {
    std::lock_guard<std::mutex> lock{mutex_};
    connection_.reset();
}

void TimescaleDbDataStorage::EnsureSchema() {
    pqxx::work tx{*connection_};
    tx.exec(kCreateTable);
    tx.exec(kCreateHypertable);
    tx.exec(kCreateIndex);
    tx.commit();
}

void TimescaleDbDataStorage::Reconnect() {
    connection_ = std::make_unique<pqxx::connection>(settings_.connection_string);
    EnsureSchema();
}

void TimescaleDbDataStorage::ReportError(const std::string& message) const {
    if (error_handler_) {
        error_handler_(settings_.collector_id + ": " + message);
    }
}

}   //data_storage
