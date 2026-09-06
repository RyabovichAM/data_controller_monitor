#ifndef SCHEME_CLIENT_H
#define SCHEME_CLIENT_H

#include <memory>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "scheme_service.grpc.pb.h"

namespace clients {

// Client of the SchemeService contract. Lives on the same address as
// ConfigClient — config-service serves both — and blocks the same way, so its
// calls belong on a worker thread rather than a drogon event loop.
class SchemeClient {
public:
    struct Result {
        bool ok{false};
        grpc::StatusCode code{grpc::StatusCode::OK};
        std::string error;
    };

    struct SchemeList : Result {
        std::vector<dcm::scheme::v1::SchemeSummary> schemes;
    };

    struct SchemeResult : Result {
        dcm::scheme::v1::Scheme scheme;
    };

    struct SaveResult : Result {
        int64_t version{0};
    };

    explicit SchemeClient(const std::string& address);

    SchemeList ListSchemes();
    SchemeResult GetScheme(const std::string& scheme_id);
    SaveResult SaveScheme(const dcm::scheme::v1::Scheme& scheme, int64_t expected_version);
    Result DeleteScheme(const std::string& scheme_id);

private:
    std::unique_ptr<dcm::scheme::v1::SchemeService::Stub> stub_;
};

}   //clients

#endif // SCHEME_CLIENT_H
