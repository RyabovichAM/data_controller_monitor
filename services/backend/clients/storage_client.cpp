#include "storage_client.h"

#include <chrono>

namespace clients {

namespace {

// The browser is on the other end of the request, so a call that hangs has to
// end by itself rather than hold a worker thread for good.
constexpr std::chrono::seconds kDeadline{5};

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

}   //clients
