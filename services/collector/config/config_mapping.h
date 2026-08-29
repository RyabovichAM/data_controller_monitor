#ifndef CONFIG_MAPPING_H
#define CONFIG_MAPPING_H

#include <string>

#include "app_domain.h"
#include "config_service.pb.h"

namespace config {

struct KafkaSettings {
    std::string brokers;
    std::string topic;
};

bool operator==(const KafkaSettings& lhs, const KafkaSettings& rhs);
bool operator!=(const KafkaSettings& lhs, const KafkaSettings& rhs);

// The transfer layer takes its settings as strings and parses them itself
// through transfer::GetBaudRateFromString and friends, so the enums of the
// contract have to be spelled exactly the way those parsers expect. This is the
// one place where the two vocabularies meet, hence the tests next to it.
//
// Throws std::invalid_argument on a config no transport can be built from —
// the caller keeps running on the previous one.
app::Settings ToTransferSettings(const dcm::config::v1::CollectorConfig& config);

KafkaSettings ToKafkaSettings(const dcm::config::v1::CollectorConfig& config);

}   //config

#endif // CONFIG_MAPPING_H
