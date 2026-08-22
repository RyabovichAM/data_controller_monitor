#include "config_client.h"

#include <chrono>

namespace clients {

namespace {

using namespace dcm::config::v1;

// A browser is waiting at the other end, so a call that hangs has to end by
// itself rather than hold a worker thread for good.
constexpr std::chrono::seconds kDeadline{5};

}   //namespace

ConfigClient::ConfigClient(const std::string& address)
    : stub_{ConfigService::NewStub(
          grpc::CreateChannel(address, grpc::InsecureChannelCredentials()))} {
}

ConfigClient::ConfigList ConfigClient::ListCollectors() {
    ListCollectorsRequest request;
    ListCollectorsResponse response;

    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + kDeadline);

    grpc::Status status = stub_->ListCollectors(&context, request, &response);

    ConfigList result;
    result.ok = status.ok();
    result.code = status.error_code();
    result.error = status.error_message();

    if (status.ok()) {
        result.configs.assign(response.configs().begin(), response.configs().end());
    }

    return result;
}

ConfigClient::SaveResult ConfigClient::SaveConfig(const CollectorConfig& config,
                                                  int64_t expected_version) {
    SaveConfigRequest request;
    *request.mutable_config() = config;
    request.set_expected_version(expected_version);

    SaveConfigResponse response;

    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + kDeadline);

    grpc::Status status = stub_->SaveConfig(&context, request, &response);

    SaveResult result;
    result.ok = status.ok();
    result.code = status.error_code();
    result.error = status.error_message();
    result.version = response.version();

    return result;
}

ConfigClient::Result ConfigClient::DeleteCollector(const std::string& collector_id) {
    DeleteCollectorRequest request;
    request.set_collector_id(collector_id);

    DeleteCollectorResponse response;

    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + kDeadline);

    grpc::Status status = stub_->DeleteCollector(&context, request, &response);

    Result result;
    result.ok = status.ok();
    result.code = status.error_code();
    result.error = status.error_message();

    return result;
}

}   //clients
