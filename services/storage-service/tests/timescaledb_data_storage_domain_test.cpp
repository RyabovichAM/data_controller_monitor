#include <stdexcept>

#include <gtest/gtest.h>

#include "timescaledb_data_storage_domain.h"

namespace {

using data_storage::FromEpochSeconds;
using data_storage::GetTimescaleDbSettings;
using data_storage::Settings;
using data_storage::ToEpochSeconds;

TEST(TimescaleDbSettings, ReadsDsnAndCollectorId) {
    Settings settings;
    settings["collector_id"] = "collector-1";
    settings["dsn"] = "postgresql://dcm:dcm@timescaledb:5432/dcm_history";
    settings["period"] = "1000";

    auto result = GetTimescaleDbSettings(settings);
    EXPECT_EQ(result.collector_id, "collector-1");
    EXPECT_EQ(result.connection_string, "postgresql://dcm:dcm@timescaledb:5432/dcm_history");
    EXPECT_EQ(result.survey_period, 1000);
}

TEST(TimescaleDbSettings, DefaultsPeriodToZero) {
    Settings settings;
    settings["collector_id"] = "collector-1";
    settings["dsn"] = "postgresql://x";

    EXPECT_EQ(GetTimescaleDbSettings(settings).survey_period, 0);
}

TEST(TimescaleDbSettings, MissingDsnIsRejected) {
    Settings settings;
    settings["collector_id"] = "collector-1";

    EXPECT_THROW(GetTimescaleDbSettings(settings), std::invalid_argument);
}

TEST(TimescaleDbSettings, MissingCollectorIdIsRejected) {
    Settings settings;
    settings["dsn"] = "postgresql://x";

    EXPECT_THROW(GetTimescaleDbSettings(settings), std::invalid_argument);
}

// The round trip is what actually matters: DataSave converts a TimePoint to
// seconds for to_timestamp(), DataLoad converts EXTRACT(EPOCH FROM ts) back.
TEST(EpochSeconds, RoundTripsWithinAMicrosecond) {
    const auto now = std::chrono::system_clock::now();

    const double seconds = ToEpochSeconds(now);
    const auto back = FromEpochSeconds(seconds);

    const auto difference = std::chrono::duration_cast<std::chrono::microseconds>(
        now > back ? now - back : back - now);
    EXPECT_LT(difference.count(), 1);
}

TEST(EpochSeconds, TheEpochItselfIsZero) {
    EXPECT_EQ(ToEpochSeconds(std::chrono::system_clock::time_point{}), 0.0);
}

}   //namespace
