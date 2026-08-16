#ifndef STORAGE_CLIENT_H
#define STORAGE_CLIENT_H

#include <memory>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "storage_service.grpc.pb.h"

namespace clients {

// Client of the StorageService contract. The generated stub is synchronous, so
// every call here blocks — they belong on a worker thread, never on a drogon
// event loop.
class StorageClient {
public:
    struct CollectorList {
        bool ok{false};
        std::string error;
        std::vector<std::string> collector_ids;
    };

    explicit StorageClient(const std::string& address);

    CollectorList ListCollectors();

private:
    std::unique_ptr<dcm::storage::v1::StorageService::Stub> stub_;
};

}   //clients

#endif // STORAGE_CLIENT_H
