#ifndef FILE_DATA_STORAGE_DOMAIN_H
#define FILE_DATA_STORAGE_DOMAIN_H

#include <ctime>
#include <optional>
#include <string>

#include "data_storage_domain.h"
#include "data_storage_interface.h"

namespace data_storage {

enum class DataFormat {
    TEXT,
    BINARY
};

struct FileDataStorageSettings : public DataStorageSettings {
    DataFormat data_format{DataFormat::TEXT};
};

// Throws std::invalid_argument on a format nobody can write.
FileDataStorageSettings GetFileDataStorageSettings(const Settings& settings);

// A file per day named after the local date, the time written inside it is
// local as well — that is what makes the files readable without a tool.
std::string FormatDate(const std::tm& day);
std::string FormatDate(TimePoint point);
std::string FormatTime(TimePoint point);

std::tm LocalTime(TimePoint point);

// Rebuilds a point out of the date its file is named after and a time from
// inside it. Empty if either of them is not what it should be.
std::optional<TimePoint> ParseTimePoint(const std::string& date, const std::string& time);

}   //data_storage

#endif // FILE_DATA_STORAGE_DOMAIN_H
