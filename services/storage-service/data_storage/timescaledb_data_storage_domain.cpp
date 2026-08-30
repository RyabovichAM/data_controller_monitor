#include "timescaledb_data_storage_domain.h"

#include <stdexcept>

namespace data_storage {

namespace {

std::string Value(const Settings& settings, const std::string& key) {
    auto it = settings.find(key);
    return it == settings.end() ? std::string{} : it->second;
}

}   //namespace

TimescaleDbSettings GetTimescaleDbSettings(const Settings& settings) {
    TimescaleDbSettings result;
    result.collector_id = Value(settings, "collector_id");
    result.connection_string = Value(settings, "dsn");

    if (result.collector_id.empty()) {
        throw std::invalid_argument("data_storage: collector_id is empty");
    }
    if (result.connection_string.empty()) {
        throw std::invalid_argument("data_storage: dsn is empty");
    }

    const std::string period = Value(settings, "period");
    result.survey_period = period.empty() ? 0 : std::stoll(period);

    return result;
}

double ToEpochSeconds(TimePoint point) {
    return std::chrono::duration<double>{point.time_since_epoch()}.count();
}

TimePoint FromEpochSeconds(double seconds) {
    return TimePoint{std::chrono::duration_cast<TimePoint::duration>(
        std::chrono::duration<double>{seconds})};
}

}   //data_storage
