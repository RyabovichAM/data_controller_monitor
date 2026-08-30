#include "timescale_collector_directory.h"

#include <utility>

namespace data_storage {

TimescaleCollectorDirectory::TimescaleCollectorDirectory(std::string connection_string)
    : connection_string_{std::move(connection_string)} {
}

bool TimescaleCollectorDirectory::HasCollector(const std::string& collector_id) const {
    std::lock_guard<std::mutex> lock{mutex_};

    try {
        pqxx::work tx{Connected()};
        pqxx::result rows = tx.exec_params(
            "SELECT 1 FROM sensor_data WHERE collector_id = $1 LIMIT 1", collector_id);
        return !rows.empty();
    } catch (const std::exception&) {
        connection_.reset();
        return false;
    }
}

std::vector<std::string> TimescaleCollectorDirectory::KnownCollectors() const {
    std::lock_guard<std::mutex> lock{mutex_};

    try {
        pqxx::work tx{Connected()};
        pqxx::result rows = tx.exec("SELECT DISTINCT collector_id FROM sensor_data "
                                    "ORDER BY collector_id");

        std::vector<std::string> collector_ids;
        collector_ids.reserve(rows.size());
        for (const auto& row : rows) {
            collector_ids.push_back(row[0].as<std::string>());
        }

        return collector_ids;
    } catch (const std::exception&) {
        connection_.reset();
        return {};
    }
}

pqxx::connection& TimescaleCollectorDirectory::Connected() const {
    if (!connection_) {
        connection_ = std::make_unique<pqxx::connection>(connection_string_);
    }

    return *connection_;
}

}   //data_storage
