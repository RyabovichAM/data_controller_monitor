#ifndef DATA_STORAGE_INTERFACE_H
#define DATA_STORAGE_INTERFACE_H

#include <chrono>
#include <functional>
#include <string>
#include <vector>

namespace data_storage {

using ErrorHandler = std::function<void(const std::string&)>;
using TimePoint = std::chrono::system_clock::time_point;

// One stored sample: when it arrived and the JSON the controller sent. The
// service never looks inside the JSON — its schema belongs to the controller.
struct DataPoint {
    TimePoint timestamp;
    std::string json;
};

// The storage of one collector. Not a template any more: the desktop
// application parameterised it over its Qt types, the service has exactly one
// pair of them.
class DataStorageInterface {
public:
    virtual ~DataStorageInterface() = default;

    virtual void SetErrorHandler(ErrorHandler handler) = 0;
    virtual void DataSave(const std::string& json) = 0;
    virtual std::vector<DataPoint> DataLoad(TimePoint from, TimePoint to) = 0;
    virtual bool Open() = 0;
    virtual bool IsOpen() const = 0;
    virtual void Close() = 0;
};

}   //data_storage

#endif // DATA_STORAGE_INTERFACE_H
