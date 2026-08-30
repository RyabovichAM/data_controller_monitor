#ifndef TIMESCALEDB_DATA_STORAGE_DOMAIN_H
#define TIMESCALEDB_DATA_STORAGE_DOMAIN_H

#include <cstdint>
#include <string>

#include "data_storage_domain.h"
#include "data_storage_interface.h"

namespace data_storage {

struct TimescaleDbSettings {
    std::string collector_id;
    std::string connection_string;
    int64_t survey_period{0};   // ms between two saved samples, 0 keeps every one
};

// Throws std::invalid_argument if "collector_id" or "dsn" is missing.
TimescaleDbSettings GetTimescaleDbSettings(const Settings& settings);

// Postgres has no chrono binding for a full time_point (only year_month_day),
// so timestamps cross the wire as seconds since the epoch — a plain double
// rather than a formatted string, which sidesteps both time zones and
// PostgreSQL's configurable text output format entirely.
double ToEpochSeconds(TimePoint point);
TimePoint FromEpochSeconds(double seconds);

}   //data_storage

#endif // TIMESCALEDB_DATA_STORAGE_DOMAIN_H
