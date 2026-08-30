#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include "data_storage_factory.h"
#include "file_data_storage.h"

namespace {

namespace fs = std::filesystem;

using data_storage::DataPoint;
using data_storage::FileDataStorage;
using data_storage::FormatDate;
using data_storage::ParseTimePoint;
using data_storage::Settings;
using data_storage::TimePoint;

// The storage hands points to a sink; the tests want them in one place.
std::vector<DataPoint> Collect(data_storage::DataStorageInterface& storage, TimePoint from,
                               TimePoint to, size_t limit = 0) {
    std::vector<DataPoint> points;
    storage.DataLoad(from, to, [&](const DataPoint& point) {
        points.push_back(point);
        return limit == 0 || points.size() < limit;
    });
    return points;
}

class FileDataStorageTest : public ::testing::Test {
protected:
    void SetUp() override {
        root_ = fs::temp_directory_path() /
                ("dcm-storage-test-" + std::to_string(::getpid()) + "-" +
                 ::testing::UnitTest::GetInstance()->current_test_info()->name());
        fs::create_directories(root_);
    }

    void TearDown() override {
        fs::remove_all(root_);
    }

    Settings SettingsFor(const std::string& format, const std::string& period = "0") const {
        Settings settings;
        settings["type"] = "file";
        settings["location"] = root_.string() + "/";
        settings["data_format"] = format;
        settings["period"] = period;
        return settings;
    }

    std::string Today() const {
        return FormatDate(std::chrono::system_clock::now());
    }

    // Writes a day file directly, so the times inside it are known and the
    // test does not depend on when it runs.
    void WriteTextDay(const std::string& date,
                      const std::vector<std::pair<std::string, std::string>>& records) const {
        std::ofstream file{root_ / (date + ".csv")};
        for (const auto& [time, json] : records) {
            file << time << ' ' << json << '\n';
        }
    }

    fs::path root_;
};

TEST_F(FileDataStorageTest, SavedSamplesAreLoadedBack) {
    FileDataStorage storage{SettingsFor("text")};
    ASSERT_TRUE(storage.Open());

    storage.DataSave(R"({"param1":"1"})");
    storage.DataSave(R"({"param1":"2"})");

    const TimePoint now = std::chrono::system_clock::now();
    std::vector<DataPoint> points =
        Collect(storage, now - std::chrono::hours{1}, now + std::chrono::hours{1});

    ASSERT_EQ(points.size(), 2u);
    EXPECT_EQ(points[0].json, R"({"param1":"1"})");
    EXPECT_EQ(points[1].json, R"({"param1":"2"})");
}

TEST_F(FileDataStorageTest, TextFilesStayReadable) {
    FileDataStorage storage{SettingsFor("text")};
    ASSERT_TRUE(storage.Open());
    storage.DataSave(R"({"param1":"1"})");

    std::ifstream file{root_ / (Today() + ".csv")};
    std::string line;
    ASSERT_TRUE(std::getline(file, line));

    // "HH:MM:SS " and the JSON exactly as it arrived — the format the desktop
    // application writes as well.
    EXPECT_EQ(line.size(), 9u + std::string{R"({"param1":"1"})"}.size());
    EXPECT_EQ(line.substr(8), R"( {"param1":"1"})");
}

TEST_F(FileDataStorageTest, BinaryFormatRoundTrips) {
    FileDataStorage storage{SettingsFor("binary")};
    ASSERT_TRUE(storage.Open());

    storage.DataSave(R"({"param1":"1"})");
    storage.DataSave(R"({"param1":"2"})");

    ASSERT_TRUE(fs::exists(root_ / (Today() + ".dat")));

    const TimePoint now = std::chrono::system_clock::now();
    std::vector<DataPoint> points =
        Collect(storage, now - std::chrono::hours{1}, now + std::chrono::hours{1});

    ASSERT_EQ(points.size(), 2u);
    EXPECT_EQ(points[1].json, R"({"param1":"2"})");
}

TEST_F(FileDataStorageTest, FirstSampleIsKeptAndTheNextOnesAreThinned) {
    FileDataStorage storage{SettingsFor("text", "60000")};
    ASSERT_TRUE(storage.Open());

    storage.DataSave(R"({"param1":"1"})");
    storage.DataSave(R"({"param1":"2"})");

    const TimePoint now = std::chrono::system_clock::now();
    std::vector<DataPoint> points =
        Collect(storage, now - std::chrono::hours{1}, now + std::chrono::hours{1});

    ASSERT_EQ(points.size(), 1u);
    EXPECT_EQ(points[0].json, R"({"param1":"1"})");
}

TEST_F(FileDataStorageTest, LoadReturnsOnlyTheRequestedWindow) {
    const std::string date = Today();
    WriteTextDay(date, {{"10:00:00", R"({"n":"early"})"},
                        {"12:00:00", R"({"n":"middle"})"},
                        {"14:00:00", R"({"n":"late"})"}});

    FileDataStorage storage{SettingsFor("text")};

    std::optional<TimePoint> from = ParseTimePoint(date, "11:00:00");
    std::optional<TimePoint> to = ParseTimePoint(date, "13:00:00");
    ASSERT_TRUE(from && to);

    std::vector<DataPoint> points = Collect(storage, *from, *to);

    ASSERT_EQ(points.size(), 1u);
    EXPECT_EQ(points[0].json, R"({"n":"middle"})");
}

TEST_F(FileDataStorageTest, LoadSpansSeveralDays) {
    // Two days apart, so the walk over the range has to step over the day in
    // between and survive a file that does not exist.
    WriteTextDay("10.08.2026", {{"23:00:00", R"({"n":"first"})"}});
    WriteTextDay("12.08.2026", {{"01:00:00", R"({"n":"third"})"}});

    FileDataStorage storage{SettingsFor("text")};

    std::optional<TimePoint> from = ParseTimePoint("10.08.2026", "00:00:00");
    std::optional<TimePoint> to = ParseTimePoint("12.08.2026", "23:59:59");
    ASSERT_TRUE(from && to);

    std::vector<DataPoint> points = Collect(storage, *from, *to);

    ASSERT_EQ(points.size(), 2u);
    EXPECT_EQ(points[0].json, R"({"n":"first"})");
    EXPECT_EQ(points[1].json, R"({"n":"third"})");
}

TEST_F(FileDataStorageTest, ReversedWindowLoadsNothing) {
    FileDataStorage storage{SettingsFor("text")};
    ASSERT_TRUE(storage.Open());
    storage.DataSave(R"({"param1":"1"})");

    const TimePoint now = std::chrono::system_clock::now();
    EXPECT_TRUE(Collect(storage, now + std::chrono::hours{1}, now).empty());
}

TEST_F(FileDataStorageTest, MissingDayIsNotAnError) {
    FileDataStorage storage{SettingsFor("text")};

    std::optional<TimePoint> from = ParseTimePoint("01.01.2020", "00:00:00");
    std::optional<TimePoint> to = ParseTimePoint("01.01.2020", "23:59:59");
    ASSERT_TRUE(from && to);

    EXPECT_TRUE(Collect(storage, *from, *to).empty());
}

TEST_F(FileDataStorageTest, UnknownFormatIsRejected) {
    EXPECT_THROW(FileDataStorage{SettingsFor("parquet")}, std::invalid_argument);
}

TEST_F(FileDataStorageTest, UnknownStorageTypeIsRejected) {
    Settings settings = SettingsFor("text");
    settings["type"] = "influxdb";   // not one this service knows how to build

    EXPECT_THROW(data_storage::DataStorageFactory::CreateDataStorage(settings),
                 std::invalid_argument);
}

TEST_F(FileDataStorageTest, TimePointsSurviveTheirOwnFormatting) {
    const TimePoint now = std::chrono::system_clock::now();

    std::optional<TimePoint> parsed =
        ParseTimePoint(FormatDate(now), data_storage::FormatTime(now));
    ASSERT_TRUE(parsed);

    // Files keep whole seconds, so that is the accuracy to expect back.
    const auto difference = std::chrono::duration_cast<std::chrono::milliseconds>(now - *parsed);
    EXPECT_LT(difference.count(), 1000);
    EXPECT_GE(difference.count(), 0);
}

TEST_F(FileDataStorageTest, TheIntervalIsHalfOpen) {
    const std::string date = Today();
    WriteTextDay(date, {{"10:00:00", R"({"n":"at from"})"},
                        {"12:00:00", R"({"n":"at to"})"}});

    FileDataStorage storage{SettingsFor("text")};

    std::optional<TimePoint> from = ParseTimePoint(date, "10:00:00");
    std::optional<TimePoint> to = ParseTimePoint(date, "12:00:00");
    ASSERT_TRUE(from && to);

    std::vector<DataPoint> points = Collect(storage, *from, *to);

    // [from, to): the point standing on from is in, the one on to is not.
    ASSERT_EQ(points.size(), 1u);
    EXPECT_EQ(points[0].json, R"({"n":"at from"})");
}

TEST_F(FileDataStorageTest, UnboundedRangeReachesEveryDayOnDisk) {
    WriteTextDay("01.01.2020", {{"00:00:01", R"({"n":"ancient"})"}});
    WriteTextDay("31.12.2030", {{"23:59:59", R"({"n":"distant"})"}});

    FileDataStorage storage{SettingsFor("text")};

    // What DataLoad gets when the request leaves both bounds unset.
    std::vector<DataPoint> points = Collect(storage, TimePoint::min(), TimePoint::max());

    ASSERT_EQ(points.size(), 2u);
    EXPECT_EQ(points[0].json, R"({"n":"ancient"})");
    EXPECT_EQ(points[1].json, R"({"n":"distant"})");
}

TEST_F(FileDataStorageTest, TheSinkStopsTheWalk) {
    const std::string date = Today();
    WriteTextDay(date, {{"10:00:00", R"({"n":"1"})"},
                        {"11:00:00", R"({"n":"2"})"},
                        {"12:00:00", R"({"n":"3"})"}});

    FileDataStorage storage{SettingsFor("text")};

    // This is what the limit of DataLoadRequest does on the server.
    std::vector<DataPoint> points = Collect(storage, TimePoint::min(), TimePoint::max(), 2);

    ASSERT_EQ(points.size(), 2u);
    EXPECT_EQ(points[1].json, R"({"n":"2"})");
}

TEST_F(FileDataStorageTest, FilesOfAnotherFormatAreIgnored) {
    // A directory that was once written in binary must not turn into records
    // with unparsable times when read as text.
    WriteTextDay(Today(), {{"10:00:00", R"({"n":"text"})"}});
    std::ofstream{root_ / (Today() + ".dat")} << "not a text day";

    FileDataStorage storage{SettingsFor("text")};
    std::vector<DataPoint> points = Collect(storage, TimePoint::min(), TimePoint::max());

    ASSERT_EQ(points.size(), 1u);
    EXPECT_EQ(points[0].json, R"({"n":"text"})");
}

}   //namespace
