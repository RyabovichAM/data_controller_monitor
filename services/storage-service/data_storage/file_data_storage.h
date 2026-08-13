#ifndef FILE_DATA_STORAGE_H
#define FILE_DATA_STORAGE_H

#include <fstream>
#include <string>
#include <vector>

#include "data_storage_interface.h"
#include "file_data_storage_domain.h"

namespace data_storage {

// Stores the samples of one collector as one file per day. Text files are
// "HH:MM:SS <json>" per line and stay readable with cat; binary files frame
// both fields with a big-endian length.
class FileDataStorage : public DataStorageInterface {
public:
    explicit FileDataStorage(const Settings& settings);

    void SetErrorHandler(ErrorHandler handler) override;
    void DataSave(const std::string& json) override;
    std::vector<DataPoint> DataLoad(TimePoint from, TimePoint to) override;
    bool Open() override;
    bool IsOpen() const override;
    void Close() override;

private:
    FileDataStorageSettings settings_;
    ErrorHandler error_handler_{nullptr};

    std::ofstream save_file_;
    std::string save_file_date_;   // the day the open file belongs to
    TimePoint last_save_;

    std::string FileName(const std::string& date) const;
    bool OpenForDate(const std::string& date);
    void AppendDay(const std::string& date, TimePoint from, TimePoint to,
                   std::vector<DataPoint>& points) const;
    void ReportError(const std::string& message) const;
};

}   //data_storage

#endif // FILE_DATA_STORAGE_H
