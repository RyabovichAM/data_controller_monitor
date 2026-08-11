#ifndef CONFIG_REPOSITORY_H
#define CONFIG_REPOSITORY_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "config_service.pb.h"

namespace config {

using dcm::config::v1::CollectorConfig;

// Storage of collector configurations. The gRPC layer talks to this interface
// only, so SQL stays out of the service implementation.
class ConfigRepository {
public:
    // The version guard belongs here: only the storage layer can compare the
    // expected version and bump the stored one under a single lock.
    struct SaveResult {
        bool conflict{false};
        int64_t version{0};   // version after the save, the stored one on conflict
    };

    virtual ~ConfigRepository() = default;

    virtual std::optional<CollectorConfig> Get(const std::string& collector_id) = 0;
    virtual std::vector<CollectorConfig> List() = 0;
    virtual SaveResult Save(const CollectorConfig& config, int64_t expected_version) = 0;
    virtual bool Delete(const std::string& collector_id) = 0;
};

}   //config

#endif // CONFIG_REPOSITORY_H
