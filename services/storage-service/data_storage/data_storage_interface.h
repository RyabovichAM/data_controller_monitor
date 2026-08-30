#ifndef DATA_STORAGE_INTERFACE_H
#define DATA_STORAGE_INTERFACE_H

#include <chrono>
#include <cstdint>
#include <functional>
#include <optional>
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

// Takes one point of a range. Returning false stops the walk — the client is
// gone, or it has had the number of points it asked for.
using DataSink = std::function<bool(const DataPoint&)>;

// The survey_period policy, shared by every backend: the first sample is
// always kept (there is nothing to compare it against yet), later ones are
// kept only survey_period milliseconds apart. Zero keeps every sample.
class SampleThinner {
public:
    explicit SampleThinner(int64_t survey_period_ms) : survey_period_ms_{survey_period_ms} {
    }

    // True if a sample arriving at `now` should be kept. Has side effects: a
    // kept sample becomes the new reference point for the next call.
    bool ShouldKeep(TimePoint now) {
        if (kept_ && std::chrono::duration_cast<std::chrono::milliseconds>(now - *kept_)
                          .count() < survey_period_ms_) {
            return false;
        }

        kept_ = now;
        return true;
    }

private:
    int64_t survey_period_ms_;
    std::optional<TimePoint> kept_;
};

// The storage of one collector. Not a template any more: the desktop
// application parameterised it over its Qt types, the service has exactly one
// pair of them.
class DataStorageInterface {
public:
    virtual ~DataStorageInterface() = default;

    virtual void SetErrorHandler(ErrorHandler handler) = 0;
    virtual void DataSave(const std::string& json) = 0;

    // Half-open [from, to), ordered by ascending timestamp — the interval the
    // StorageService contract is written in. Points go out through the sink
    // rather than in a container: a range may span months, and neither this
    // service nor its client should hold one in memory. May run while another
    // thread is saving: the gRPC server reads what the Kafka consumer writes.
    virtual void DataLoad(TimePoint from, TimePoint to, const DataSink& sink) = 0;
    virtual bool Open() = 0;
    virtual bool IsOpen() const = 0;
    virtual void Close() = 0;
};

}   //data_storage

#endif // DATA_STORAGE_INTERFACE_H
