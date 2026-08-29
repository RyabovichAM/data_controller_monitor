#include "config_mapping.h"

#include <stdexcept>

namespace config {

namespace {

using namespace dcm::config::v1;

// Baud rate and data bits are the easy half: the contract keeps their enum
// values equal to the physical ones, so the number itself is the string the
// parser expects. IsValid guards against a value from a newer contract.
std::string BaudRateToString(BaudRate baud_rate) {
    if (baud_rate == BAUD_RATE_UNSPECIFIED || !BaudRate_IsValid(baud_rate)) {
        throw std::invalid_argument("config: unusable baud_rate");
    }

    return std::to_string(static_cast<int>(baud_rate));
}

std::string DataBitsToString(DataBits data_bits) {
    if (data_bits == DATA_BITS_UNSPECIFIED || !DataBits_IsValid(data_bits)) {
        throw std::invalid_argument("config: unusable data_bits");
    }

    return std::to_string(static_cast<int>(data_bits));
}

// The rest have their own numbering — in Qt zero is a meaningful value, in
// proto3 it means "not set" — so they are spelled out.
std::string ParityToString(Parity parity) {
    switch (parity) {
    case PARITY_NONE:
        return "None";
    case PARITY_EVEN:
        return "Even";
    case PARITY_ODD:
        return "Odd";
    case PARITY_SPACE:
        return "Space";
    case PARITY_MARK:
        return "Mark";
    default:
        throw std::invalid_argument("config: unusable parity");
    }
}

std::string StopBitsToString(StopBits stop_bits) {
    switch (stop_bits) {
    case STOP_BITS_ONE:
        return "1";
    case STOP_BITS_ONE_AND_HALF:
        return "1.5";
    case STOP_BITS_TWO:
        return "2";
    default:
        throw std::invalid_argument("config: unusable stop_bits");
    }
}

std::string FlowControlToString(FlowControl flow_control) {
    switch (flow_control) {
    case FLOW_CONTROL_NONE:
        return "None";
    case FLOW_CONTROL_HARDWARE:
        return "Hardware";
    case FLOW_CONTROL_SOFTWARE:
        return "Software";
    default:
        throw std::invalid_argument("config: unusable flow_control");
    }
}

}   //namespace

bool operator==(const KafkaSettings& lhs, const KafkaSettings& rhs) {
    return lhs.brokers == rhs.brokers && lhs.topic == rhs.topic;
}

bool operator!=(const KafkaSettings& lhs, const KafkaSettings& rhs) {
    return !(lhs == rhs);
}

app::Settings ToTransferSettings(const CollectorConfig& config) {
    app::Settings settings;

    switch (config.transfer().kind_case()) {
    case dcm::config::v1::TransferSettings::kSerial: {
        const SerialTransfer& serial = config.transfer().serial();
        // "Serial" and "TCP/IP" are what transfer::TransferFactory switches on.
        settings["type"] = "Serial";
        settings["port_name"] = serial.port_name();
        settings["baud_rate"] = BaudRateToString(serial.baud_rate());
        settings["data_bits"] = DataBitsToString(serial.data_bits());
        settings["parity"] = ParityToString(serial.parity());
        settings["stop_bits"] = StopBitsToString(serial.stop_bits());
        settings["flow_control"] = FlowControlToString(serial.flow_control());
        break;
    }
    case dcm::config::v1::TransferSettings::kTcpIp: {
        const TcpIpTransfer& tcp_ip = config.transfer().tcp_ip();
        settings["type"] = "TCP/IP";
        settings["host"] = tcp_ip.host();
        settings["port"] = std::to_string(tcp_ip.port());
        break;
    }
    case dcm::config::v1::TransferSettings::KIND_NOT_SET:
        throw std::invalid_argument("config: transfer is not set");
    }

    return settings;
}

KafkaSettings ToKafkaSettings(const CollectorConfig& config) {
    KafkaSettings settings;
    settings.brokers = config.kafka().brokers();
    settings.topic = config.kafka().topic();

    if (settings.brokers.empty() || settings.topic.empty()) {
        throw std::invalid_argument("config: kafka settings are incomplete");
    }

    return settings;
}

}   //config
