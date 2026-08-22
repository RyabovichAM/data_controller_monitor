#ifndef CONFIG_CLIENT_H
#define CONFIG_CLIENT_H

#include <memory>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "config_service.grpc.pb.h"

namespace clients {

// Client of the ConfigService contract — the side of it the collector never
// touches: it only reads its own config, everything here writes.
//
// The generated stub is synchronous, so every call blocks and belongs on a
// worker thread rather than on a drogon event loop.
class ConfigClient {
public:
    // The gRPC status code travels out unchanged: the REST layer turns it into
    // 409 for a version conflict, 404 for a missing collector and so on.
    struct Result {
        bool ok{false};
        grpc::StatusCode code{grpc::StatusCode::OK};
        std::string error;
    };

    struct ConfigList : Result {
        std::vector<dcm::config::v1::CollectorConfig> configs;
    };

    struct SaveResult : Result {
        int64_t version{0};
    };

    explicit ConfigClient(const std::string& address);

    ConfigList ListCollectors();

    // expected_version is the one the editor started from; zero overwrites
    // unconditionally, which is what creating a new config does.
    SaveResult SaveConfig(const dcm::config::v1::CollectorConfig& config,
                          int64_t expected_version);

    Result DeleteCollector(const std::string& collector_id);

private:
    std::unique_ptr<dcm::config::v1::ConfigService::Stub> stub_;
};

}   //clients

#endif // CONFIG_CLIENT_H
