#include "file_data_storage.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <utility>

namespace data_storage {

namespace {

// A framed field cannot be larger than this. Without the guard a corrupt file
// would have the reader allocate whatever the four bytes happened to say.
constexpr uint32_t kMaxFieldSize = 16 * 1024 * 1024;

// Four bytes of big-endian length, then the bytes themselves, both fields UTF-8.
// The Qt version wrote QDataStream's UTF-16 framing here; nothing outside this
// service ever read those files, so the plainer layout wins.
void WriteFramed(std::ostream& out, const std::string& value) {
    const uint32_t size = static_cast<uint32_t>(value.size());
    const char header[4] = {static_cast<char>((size >> 24) & 0xFF),
                            static_cast<char>((size >> 16) & 0xFF),
                            static_cast<char>((size >> 8) & 0xFF),
                            static_cast<char>(size & 0xFF)};

    out.write(header, sizeof(header));
    out.write(value.data(), size);
}

bool ReadFramed(std::istream& in, std::string& value) {
    unsigned char header[4];
    if (!in.read(reinterpret_cast<char*>(header), sizeof(header))) {
        return false;
    }

    const uint32_t size = (static_cast<uint32_t>(header[0]) << 24) |
                          (static_cast<uint32_t>(header[1]) << 16) |
                          (static_cast<uint32_t>(header[2]) << 8) |
                          static_cast<uint32_t>(header[3]);
    if (size > kMaxFieldSize) {
        return false;
    }

    value.resize(size);
    return static_cast<bool>(in.read(value.data(), size));
}

bool ReadRecord(std::istream& in, DataFormat format, std::string& time, std::string& json) {
    if (format == DataFormat::BINARY) {
        return ReadFramed(in, time) && ReadFramed(in, json);
    }

    std::string line;
    if (!std::getline(in, line)) {
        return false;
    }

    const size_t separator = line.find(' ');
    if (separator == std::string::npos) {
        return false;
    }

    time = line.substr(0, separator);
    json = line.substr(separator + 1);

    return true;
}

}   //namespace

FileDataStorage::FileDataStorage(const Settings& settings)
    : settings_{GetFileDataStorageSettings(settings)}
    , thinner_{settings_.survey_period} {
}

void FileDataStorage::SetErrorHandler(ErrorHandler handler) {
    error_handler_ = handler;
}

void FileDataStorage::DataSave(const std::string& json) {
    const TimePoint now = std::chrono::system_clock::now();
    if (!thinner_.ShouldKeep(now)) {
        return;
    }

    // Rolled over before the write, not after it, so a sample arriving right
    // after midnight lands in the file of its own day.
    const std::string date = FormatDate(now);
    if (date != save_file_date_ && !OpenForDate(date)) {
        return;
    }

    switch (settings_.data_format) {
    case DataFormat::TEXT:
        save_file_ << FormatTime(now) << ' ' << json << '\n';
        break;
    case DataFormat::BINARY:
        WriteFramed(save_file_, FormatTime(now));
        WriteFramed(save_file_, json);
        break;
    }

    // Flushed on every sample: a killed container must not take the last
    // minutes of data with it.
    save_file_.flush();
    if (!save_file_) {
        ReportError("cannot write to " + FileName(date));
    }
}

void FileDataStorage::DataLoad(TimePoint from, TimePoint to, const DataSink& sink) {
    if (from >= to || !sink) {
        return;
    }

    // The days come from what is on disk rather than from walking the range:
    // a request may start before the first file ever written, and "since the
    // beginning of history" would then mean thousands of misses.
    std::vector<std::pair<TimePoint, std::string>> days;

    std::error_code error;
    for (const auto& entry :
         std::filesystem::directory_iterator{settings_.place_of_save, error}) {
        if (entry.path().extension() != Extension()) {
            continue;
        }

        const std::string date = entry.path().stem().string();
        std::optional<TimePoint> day_start = ParseTimePoint(date, "00:00:00");
        if (!day_start) {
            continue;
        }

        // A coarse filter — the exact one runs per record. 25 hours because a
        // day is longer than 24 when the clocks go back.
        if (*day_start >= to || *day_start + std::chrono::hours{25} <= from) {
            continue;
        }

        days.emplace_back(*day_start, date);
    }

    std::sort(days.begin(), days.end());

    for (const auto& [day_start, date] : days) {
        if (!ReadDay(date, from, to, sink)) {
            return;
        }
    }
}

bool FileDataStorage::Open() {
    return OpenForDate(FormatDate(std::chrono::system_clock::now()));
}

bool FileDataStorage::IsOpen() const {
    return save_file_.is_open();
}

void FileDataStorage::Close() {
    save_file_.close();
    save_file_date_.clear();
}

std::string FileDataStorage::Extension() const {
    return settings_.data_format == DataFormat::TEXT ? ".csv" : ".dat";
}

std::string FileDataStorage::FileName(const std::string& date) const {
    return settings_.place_of_save + date + Extension();
}

bool FileDataStorage::OpenForDate(const std::string& date) {
    save_file_.close();
    save_file_.clear();
    save_file_.open(FileName(date), std::ios::out | std::ios::app | std::ios::binary);

    if (!save_file_.is_open()) {
        ReportError("cannot open " + FileName(date));
        save_file_date_.clear();
        return false;
    }

    save_file_date_ = date;
    return true;
}

bool FileDataStorage::ReadDay(const std::string& date, TimePoint from, TimePoint to,
                              const DataSink& sink) const {
    std::ifstream file{FileName(date), std::ios::in | std::ios::binary};
    if (!file.is_open()) {
        return true;   // a day nothing was written on is not an error
    }

    std::string time;
    std::string json;
    while (ReadRecord(file, settings_.data_format, time, json)) {
        std::optional<TimePoint> point = ParseTimePoint(date, time);
        if (!point || *point < from || *point >= to) {
            continue;
        }

        if (!sink(DataPoint{*point, json})) {
            return false;
        }
    }

    return true;
}

void FileDataStorage::ReportError(const std::string& message) const {
    if (error_handler_) {
        error_handler_(message);
    }
}

}   //data_storage
