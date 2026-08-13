#include "file_data_storage_domain.h"

#include <cstdio>
#include <stdexcept>

namespace data_storage {

namespace {

constexpr const char* kDateFormat = "%d.%m.%Y";
constexpr const char* kTimeFormat = "%H:%M:%S";

std::string Value(const Settings& settings, const std::string& key) {
    auto it = settings.find(key);
    return it == settings.end() ? std::string{} : it->second;
}

DataFormat GetDataFormat(const std::string& format) {
    if (format == "text") {
        return DataFormat::TEXT;
    }
    if (format == "binary") {
        return DataFormat::BINARY;
    }

    throw std::invalid_argument("data_storage: unknown data format '" + format + "'");
}

std::string Format(const std::tm& time, const char* format) {
    char buffer[32];
    const size_t written = std::strftime(buffer, sizeof(buffer), format, &time);
    return std::string{buffer, written};
}

}   //namespace

FileDataStorageSettings GetFileDataStorageSettings(const Settings& settings) {
    FileDataStorageSettings result;
    static_cast<DataStorageSettings&>(result) = GetDataStorageSettings(settings);
    result.data_format = GetDataFormat(Value(settings, "data_format"));

    return result;
}

std::tm LocalTime(TimePoint point) {
    const std::time_t seconds = std::chrono::system_clock::to_time_t(point);

    std::tm result{};
    localtime_r(&seconds, &result);

    return result;
}

std::string FormatDate(const std::tm& day) {
    return Format(day, kDateFormat);
}

std::string FormatDate(TimePoint point) {
    return Format(LocalTime(point), kDateFormat);
}

std::string FormatTime(TimePoint point) {
    return Format(LocalTime(point), kTimeFormat);
}

std::optional<TimePoint> ParseTimePoint(const std::string& date, const std::string& time) {
    std::tm parsed{};
    int day = 0;
    int month = 0;
    int year = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;

    if (std::sscanf(date.c_str(), "%d.%d.%d", &day, &month, &year) != 3) {
        return std::nullopt;
    }
    if (std::sscanf(time.c_str(), "%d:%d:%d", &hour, &minute, &second) != 3) {
        return std::nullopt;
    }

    parsed.tm_mday = day;
    parsed.tm_mon = month - 1;
    parsed.tm_year = year - 1900;
    parsed.tm_hour = hour;
    parsed.tm_min = minute;
    parsed.tm_sec = second;
    // Local time, and let mktime work out whether DST was on that day.
    parsed.tm_isdst = -1;

    const std::time_t seconds = std::mktime(&parsed);
    if (seconds == -1) {
        return std::nullopt;
    }

    return std::chrono::system_clock::from_time_t(seconds);
}

}   //data_storage
