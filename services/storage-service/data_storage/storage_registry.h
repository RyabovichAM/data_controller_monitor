#ifndef STORAGE_REGISTRY_H
#define STORAGE_REGISTRY_H

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "data_storage_interface.h"

namespace data_storage {

// One storage per collector, built when its first message arrives: the service
// learns about collectors from the topic, not from a configuration.
//
// Shared by the Kafka consumer, which writes, and the gRPC threads, which read,
// so the map is guarded. The storages themselves are not: writing touches only
// the writer's own state, reading opens files of its own.
class StorageRegistry {
public:
    StorageRegistry(std::string root, std::string data_format, std::string period);

    // For the writing side: creates the directory and the storage if this is
    // the first message of that collector.
    DataStorageInterface* ForCollector(const std::string& collector_id);

    // For the reading side: never creates anything, so a request naming a
    // collector nobody ever heard of leaves no empty directory behind.
    DataStorageInterface* FindCollector(const std::string& collector_id);

    // Collectors that have something in storage — read off the disk rather than
    // off the map, so a restart does not lose the ones that are quiet now.
    std::vector<std::string> KnownCollectors() const;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::unique_ptr<DataStorageInterface>> storages_;

    std::string root_;
    std::string data_format_;
    std::string period_;

    // Expects the lock to be held. Opening is for the writing side only: it
    // creates today's file, which a reader has no business doing.
    DataStorageInterface* Create(const std::string& collector_id, bool open_for_writing);
};

}   //data_storage

#endif // STORAGE_REGISTRY_H
