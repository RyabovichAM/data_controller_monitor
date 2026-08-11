#include "config_service_impl.h"

#include <exception>

namespace config {

namespace {

using dcm::config::v1::TransferSettings;

// A disconnected client is only visible to the thread serving the RPC, so the
// wait for an update has to end regularly even when nothing changes.
constexpr std::chrono::milliseconds kWatchPoll{100};

grpc::Status Invalid(const std::string& message) {
    return grpc::Status{grpc::StatusCode::INVALID_ARGUMENT, message};
}

grpc::Status ValidateSerial(const dcm::config::v1::SerialTransfer& serial) {
    if (serial.port_name().empty()) {
        return Invalid("serial.port_name is empty");
    }

    // Unspecified is not a value the collector can act on: in the contract zero
    // means "not set", not "keep the default".
    if (serial.baud_rate() == dcm::config::v1::BAUD_RATE_UNSPECIFIED) {
        return Invalid("serial.baud_rate is not set");
    }
    if (serial.data_bits() == dcm::config::v1::DATA_BITS_UNSPECIFIED) {
        return Invalid("serial.data_bits is not set");
    }
    if (serial.parity() == dcm::config::v1::PARITY_UNSPECIFIED) {
        return Invalid("serial.parity is not set");
    }
    if (serial.stop_bits() == dcm::config::v1::STOP_BITS_UNSPECIFIED) {
        return Invalid("serial.stop_bits is not set");
    }
    if (serial.flow_control() == dcm::config::v1::FLOW_CONTROL_UNSPECIFIED) {
        return Invalid("serial.flow_control is not set");
    }

    return grpc::Status::OK;
}

grpc::Status ValidateTcpIp(const dcm::config::v1::TcpIpTransfer& tcp_ip) {
    if (tcp_ip.host().empty()) {
        return Invalid("tcp_ip.host is empty");
    }
    // The field is uint32 because proto3 has no 16-bit integers.
    if (tcp_ip.port() == 0 || tcp_ip.port() > 65535) {
        return Invalid("tcp_ip.port is out of range");
    }

    return grpc::Status::OK;
}

// Rejects what the collector could not start with. Saving such a config would
// only move the failure to the collector, where nobody is waiting for an answer.
grpc::Status Validate(const CollectorConfig& config) {
    if (config.collector_id().empty()) {
        return Invalid("collector_id is empty");
    }

    switch (config.transfer().kind_case()) {
    case TransferSettings::kSerial:
        if (grpc::Status status = ValidateSerial(config.transfer().serial()); !status.ok()) {
            return status;
        }
        break;
    case TransferSettings::kTcpIp:
        if (grpc::Status status = ValidateTcpIp(config.transfer().tcp_ip()); !status.ok()) {
            return status;
        }
        break;
    case TransferSettings::KIND_NOT_SET:
        return Invalid("transfer is not set");
    }

    if (config.kafka().brokers().empty()) {
        return Invalid("kafka.brokers is empty");
    }
    if (config.kafka().topic().empty()) {
        return Invalid("kafka.topic is empty");
    }

    return grpc::Status::OK;
}

grpc::Status Failed(const std::exception& error) {
    return grpc::Status{grpc::StatusCode::INTERNAL, error.what()};
}

}   //namespace

ConfigServiceImpl::ConfigServiceImpl(ConfigRepository& repository, WatchRegistry& watchers)
    : repository_{repository}
    , watchers_{watchers} {
}

grpc::Status ConfigServiceImpl::GetConfig(grpc::ServerContext* /*context*/,
                                          const dcm::config::v1::GetConfigRequest* request,
                                          CollectorConfig* response) {
    if (request->collector_id().empty()) {
        return Invalid("collector_id is empty");
    }

    try {
        std::optional<CollectorConfig> config = repository_.Get(request->collector_id());
        if (!config) {
            return grpc::Status{grpc::StatusCode::NOT_FOUND, "no config for " + request->collector_id()};
        }

        *response = std::move(*config);
        return grpc::Status::OK;
    } catch (const std::exception& error) {
        return Failed(error);
    }
}

grpc::Status ConfigServiceImpl::WatchConfig(grpc::ServerContext* context,
                                            const dcm::config::v1::WatchConfigRequest* request,
                                            grpc::ServerWriter<CollectorConfig>* writer) {
    if (request->collector_id().empty()) {
        return Invalid("collector_id is empty");
    }

    // Subscribed before the first read: a save landing in between would
    // otherwise be lost by both paths.
    WatchRegistry::Subscription subscription{watchers_, request->collector_id()};

    try {
        std::optional<CollectorConfig> current = repository_.Get(request->collector_id());
        // An unknown collector keeps the stream open instead of failing: it may
        // simply have started before anyone configured it.
        if (current && current->version() > request->known_version()) {
            if (!writer->Write(*current)) {
                return grpc::Status::OK;
            }
            subscription.Subscriber().MarkDelivered(current->version());
        }
    } catch (const std::exception& error) {
        return Failed(error);
    }

    while (!context->IsCancelled()) {
        CollectorConfig config;
        switch (subscription.Subscriber().Wait(kWatchPoll, &config)) {
        case WatchSubscriber::Event::kTimeout:
            break;
        case WatchSubscriber::Event::kConfig:
            if (!writer->Write(config)) {
                return grpc::Status::OK;
            }
            subscription.Subscriber().MarkDelivered(config.version());
            break;
        case WatchSubscriber::Event::kDeleted:
            return grpc::Status{grpc::StatusCode::NOT_FOUND,
                                request->collector_id() + " has been deleted"};
        }
    }

    return grpc::Status::OK;
}

grpc::Status ConfigServiceImpl::SaveConfig(grpc::ServerContext* /*context*/,
                                           const dcm::config::v1::SaveConfigRequest* request,
                                           dcm::config::v1::SaveConfigResponse* response) {
    if (grpc::Status status = Validate(request->config()); !status.ok()) {
        return status;
    }

    try {
        ConfigRepository::SaveResult result =
            repository_.Save(request->config(), request->expected_version());

        if (result.conflict) {
            return grpc::Status{grpc::StatusCode::FAILED_PRECONDITION,
                                "config has version " + std::to_string(result.version) +
                                    ", expected " + std::to_string(request->expected_version())};
        }

        response->set_version(result.version);

        // Watchers are told what was actually stored, version included.
        CollectorConfig stored = request->config();
        stored.set_version(result.version);
        watchers_.NotifyChanged(stored);

        return grpc::Status::OK;
    } catch (const std::exception& error) {
        return Failed(error);
    }
}

grpc::Status ConfigServiceImpl::ListCollectors(grpc::ServerContext* /*context*/,
                                               const dcm::config::v1::ListCollectorsRequest* /*request*/,
                                               dcm::config::v1::ListCollectorsResponse* response) {
    try {
        for (CollectorConfig& config : repository_.List()) {
            *response->add_configs() = std::move(config);
        }

        return grpc::Status::OK;
    } catch (const std::exception& error) {
        return Failed(error);
    }
}

grpc::Status ConfigServiceImpl::DeleteCollector(grpc::ServerContext* /*context*/,
                                                const dcm::config::v1::DeleteCollectorRequest* request,
                                                dcm::config::v1::DeleteCollectorResponse* /*response*/) {
    if (request->collector_id().empty()) {
        return Invalid("collector_id is empty");
    }

    try {
        if (!repository_.Delete(request->collector_id())) {
            return grpc::Status{grpc::StatusCode::NOT_FOUND,
                                "no config for " + request->collector_id()};
        }

        // Watching collectors are cut off rather than left waiting for a config
        // that will never come.
        watchers_.NotifyDeleted(request->collector_id());

        return grpc::Status::OK;
    } catch (const std::exception& error) {
        return Failed(error);
    }
}

}   //config
