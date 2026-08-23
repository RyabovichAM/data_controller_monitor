#include "storage_client.h"

#include <chrono>

namespace clients {

namespace {

// The browser is on the other end of the request, so a call that hangs has to
// end by itself rather than hold a worker thread for good.
constexpr std::chrono::seconds kDeadline{5};

// A range of months is a long read even when the storage is healthy, so the
// stream gets a deadline of its own rather than the unary one.
constexpr std::chrono::seconds kStreamDeadline{30};

}   //namespace

StorageClient::StorageClient(const std::string& address)
    : stub_{dcm::storage::v1::StorageService::NewStub(
          grpc::CreateChannel(address, grpc::InsecureChannelCredentials()))} {
}

StorageClient::CollectorList StorageClient::ListCollectors() {
    dcm::storage::v1::ListCollectorsRequest request;
    dcm::storage::v1::ListCollectorsResponse response;

    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + kDeadline);

    grpc::Status status = stub_->ListCollectors(&context, request, &response);
    if (!status.ok()) {
        return CollectorList{false, status.error_message(), {}};
    }

    CollectorList result;
    result.ok = true;
    result.collector_ids.assign(response.collector_ids().begin(),
                                response.collector_ids().end());

    return result;
}

StorageClient::History StorageClient::DataLoad(const std::string& collector_id,
                                               std::optional<int64_t> from_seconds,
                                               std::optional<int64_t> to_seconds,
                                               uint32_t limit) {
    dcm::storage::v1::DataLoadRequest request;
    request.set_collector_id(collector_id);
    request.set_limit(limit);

    // Left unset, a bound means "since the beginning" or "up to now" — that is
    // what the contract says, so absent parameters are simply not written.
    if (from_seconds) {
        request.mutable_from()->set_seconds(*from_seconds);
    }
    if (to_seconds) {
        request.mutable_to()->set_seconds(*to_seconds);
    }

    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + kStreamDeadline);

    std::unique_ptr<grpc::ClientReader<dcm::storage::v1::DataPoint>> reader =
        stub_->DataLoad(&context, request);

    History history;
    dcm::storage::v1::DataPoint point;
    while (reader->Read(&point)) {
        history.points.push_back(
            HistoryPoint{point.timestamp().seconds(), point.payload_json()});
    }

    grpc::Status status = reader->Finish();
    history.ok = status.ok();
    history.code = status.error_code();
    history.error = status.error_message();

    return history;
}

}   //clients
