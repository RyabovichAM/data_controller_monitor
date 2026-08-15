#include "storage_service_impl.h"

#include <chrono>
#include <cstdint>

namespace storage {

namespace {

using data_storage::TimePoint;

TimePoint ToTimePoint(const google::protobuf::Timestamp& timestamp) {
    const auto since_epoch = std::chrono::seconds{timestamp.seconds()} +
                             std::chrono::nanoseconds{timestamp.nanos()};

    return TimePoint{std::chrono::duration_cast<TimePoint::duration>(since_epoch)};
}

google::protobuf::Timestamp ToTimestamp(TimePoint point) {
    const auto since_epoch = point.time_since_epoch();
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(since_epoch);
    const auto nanos =
        std::chrono::duration_cast<std::chrono::nanoseconds>(since_epoch - seconds);

    google::protobuf::Timestamp timestamp;
    timestamp.set_seconds(seconds.count());
    timestamp.set_nanos(static_cast<int32_t>(nanos.count()));

    return timestamp;
}

}   //namespace

StorageServiceImpl::StorageServiceImpl(data_storage::StorageRegistry& storages)
    : storages_{storages} {
}

grpc::Status StorageServiceImpl::DataLoad(
    grpc::ServerContext* /*context*/, const dcm::storage::v1::DataLoadRequest* request,
    grpc::ServerWriter<dcm::storage::v1::DataPoint>* writer) {
    if (request->collector_id().empty()) {
        return grpc::Status{grpc::StatusCode::INVALID_ARGUMENT, "collector_id is empty"};
    }

    // An unset bound means the end of history it stands at, as the contract says.
    const TimePoint from =
        request->has_from() ? ToTimePoint(request->from()) : TimePoint::min();
    const TimePoint to = request->has_to() ? ToTimePoint(request->to()) : TimePoint::max();

    if (from >= to) {
        return grpc::Status{grpc::StatusCode::INVALID_ARGUMENT, "from is not before to"};
    }

    data_storage::DataStorageInterface* storage =
        storages_.FindCollector(request->collector_id());
    if (!storage) {
        return grpc::Status{grpc::StatusCode::NOT_FOUND,
                            "nothing stored for " + request->collector_id()};
    }

    const uint32_t limit = request->limit();
    uint32_t written = 0;

    try {
        storage->DataLoad(from, to, [&](const data_storage::DataPoint& point) {
            dcm::storage::v1::DataPoint message;
            *message.mutable_timestamp() = ToTimestamp(point.timestamp);
            message.set_payload_json(point.json);

            if (!writer->Write(message)) {
                return false;   // the client is gone
            }

            ++written;
            return limit == 0 || written < limit;
        });
    } catch (const std::exception& error) {
        return grpc::Status{grpc::StatusCode::INTERNAL, error.what()};
    }

    return grpc::Status::OK;
}

grpc::Status StorageServiceImpl::ListCollectors(
    grpc::ServerContext* /*context*/,
    const dcm::storage::v1::ListCollectorsRequest* /*request*/,
    dcm::storage::v1::ListCollectorsResponse* response) {
    try {
        for (const std::string& collector_id : storages_.KnownCollectors()) {
            response->add_collector_ids(collector_id);
        }
    } catch (const std::exception& error) {
        return grpc::Status{grpc::StatusCode::INTERNAL, error.what()};
    }

    return grpc::Status::OK;
}

}   //storage
