#ifndef STORAGE_SERVICE_IMPL_H
#define STORAGE_SERVICE_IMPL_H

#include <grpcpp/grpcpp.h>

#include "data_storage/storage_registry.h"
#include "storage_service.grpc.pb.h"

namespace storage {

// Serves the history to the backend. Real time does not pass through here: the
// backend takes it straight from Kafka.
class StorageServiceImpl final : public dcm::storage::v1::StorageService::Service {
public:
    explicit StorageServiceImpl(data_storage::StorageRegistry& storages);

    grpc::Status DataLoad(grpc::ServerContext* context,
                          const dcm::storage::v1::DataLoadRequest* request,
                          grpc::ServerWriter<dcm::storage::v1::DataPoint>* writer) override;

    grpc::Status ListCollectors(grpc::ServerContext* context,
                                const dcm::storage::v1::ListCollectorsRequest* request,
                                dcm::storage::v1::ListCollectorsResponse* response) override;

private:
    data_storage::StorageRegistry& storages_;
};

}   //storage

#endif // STORAGE_SERVICE_IMPL_H
