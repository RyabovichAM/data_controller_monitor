#ifndef CONFIG_SERVICE_IMPL_H
#define CONFIG_SERVICE_IMPL_H

#include <grpcpp/grpcpp.h>

#include "config_service.grpc.pb.h"
#include "service/watch_registry.h"
#include "storage/config_repository.h"

namespace config {

// Implementation of the ConfigService contract. Holds no state of its own: the
// configs live in the repository, the live streams in the registry.
class ConfigServiceImpl final : public dcm::config::v1::ConfigService::Service {
public:
    ConfigServiceImpl(ConfigRepository& repository, WatchRegistry& watchers);

    grpc::Status GetConfig(grpc::ServerContext* context,
                           const dcm::config::v1::GetConfigRequest* request,
                           CollectorConfig* response) override;

    grpc::Status WatchConfig(grpc::ServerContext* context,
                             const dcm::config::v1::WatchConfigRequest* request,
                             grpc::ServerWriter<CollectorConfig>* writer) override;

    grpc::Status SaveConfig(grpc::ServerContext* context,
                            const dcm::config::v1::SaveConfigRequest* request,
                            dcm::config::v1::SaveConfigResponse* response) override;

    grpc::Status ListCollectors(grpc::ServerContext* context,
                                const dcm::config::v1::ListCollectorsRequest* request,
                                dcm::config::v1::ListCollectorsResponse* response) override;

    grpc::Status DeleteCollector(grpc::ServerContext* context,
                                 const dcm::config::v1::DeleteCollectorRequest* request,
                                 dcm::config::v1::DeleteCollectorResponse* response) override;

private:
    ConfigRepository& repository_;
    WatchRegistry& watchers_;
};

}   //config

#endif // CONFIG_SERVICE_IMPL_H
