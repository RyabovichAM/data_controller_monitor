#include "scheme_client.h"

#include <chrono>

namespace clients {

namespace {

using namespace dcm::scheme::v1;

// A browser is waiting at the other end, so a call that hangs has to end by
// itself rather than hold a worker thread for good.
constexpr std::chrono::seconds kDeadline{5};

}   //namespace

SchemeClient::SchemeClient(const std::string& address)
    : stub_{SchemeService::NewStub(
          grpc::CreateChannel(address, grpc::InsecureChannelCredentials()))} {
}

SchemeClient::SchemeList SchemeClient::ListSchemes() {
    ListSchemesRequest request;
    ListSchemesResponse response;

    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + kDeadline);

    grpc::Status status = stub_->ListSchemes(&context, request, &response);

    SchemeList result;
    result.ok = status.ok();
    result.code = status.error_code();
    result.error = status.error_message();

    if (status.ok()) {
        result.schemes.assign(response.schemes().begin(), response.schemes().end());
    }

    return result;
}

SchemeClient::SchemeResult SchemeClient::GetScheme(const std::string& scheme_id) {
    GetSchemeRequest request;
    request.set_scheme_id(scheme_id);

    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + kDeadline);

    SchemeResult result;
    grpc::Status status = stub_->GetScheme(&context, request, &result.scheme);

    result.ok = status.ok();
    result.code = status.error_code();
    result.error = status.error_message();

    return result;
}

SchemeClient::SaveResult SchemeClient::SaveScheme(const Scheme& scheme,
                                                  int64_t expected_version) {
    SaveSchemeRequest request;
    *request.mutable_scheme() = scheme;
    request.set_expected_version(expected_version);

    SaveSchemeResponse response;

    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + kDeadline);

    grpc::Status status = stub_->SaveScheme(&context, request, &response);

    SaveResult result;
    result.ok = status.ok();
    result.code = status.error_code();
    result.error = status.error_message();
    result.version = response.version();

    return result;
}

SchemeClient::Result SchemeClient::DeleteScheme(const std::string& scheme_id) {
    DeleteSchemeRequest request;
    request.set_scheme_id(scheme_id);

    DeleteSchemeResponse response;

    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + kDeadline);

    grpc::Status status = stub_->DeleteScheme(&context, request, &response);

    Result result;
    result.ok = status.ok();
    result.code = status.error_code();
    result.error = status.error_message();

    return result;
}

}   //clients
