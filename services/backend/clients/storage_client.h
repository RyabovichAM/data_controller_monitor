#ifndef STORAGE_CLIENT_H
#define STORAGE_CLIENT_H

#include <cstdint>
#include <memory>
#include <optional>
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

    // One sample as it left the controller: the time it was received and the
    // JSON body, untouched.
    struct HistoryPoint {
        int64_t seconds{0};
        std::string json;
    };

    struct History {
        bool ok{false};
        grpc::StatusCode code{grpc::StatusCode::OK};
        std::string error;
        std::vector<HistoryPoint> points;
    };

    explicit StorageClient(const std::string& address);

    CollectorList ListCollectors();

    // Reads the DataLoad stream to the end. Unset bounds mean the ends of
    // history, limit zero means everything the storage has — the REST layer
    // puts a ceiling on both so a browser cannot ask for a year by accident.
    History DataLoad(const std::string& collector_id, std::optional<int64_t> from_seconds,
                     std::optional<int64_t> to_seconds, uint32_t limit);

private:
    std::unique_ptr<dcm::storage::v1::StorageService::Stub> stub_;
};

}   //clients

#endif // STORAGE_CLIENT_H
