#include <stdexcept>

#include <gtest/gtest.h>

#include "config_mapping.h"
#include "serial_transfer_domain.h"
#include "tcpip_transfer_domain.h"

using namespace dcm::config::v1;

namespace {

CollectorConfig MakeConfig() {
    CollectorConfig config;
    config.set_collector_id("collector-1");
    config.mutable_kafka()->set_brokers("kafka:9092");
    config.mutable_kafka()->set_topic("sensor-data");
    return config;
}

CollectorConfig MakeSerialConfig() {
    CollectorConfig config = MakeConfig();
    SerialTransfer* serial = config.mutable_transfer()->mutable_serial();
    serial->set_port_name("/dev/ttyUSB0");
    serial->set_baud_rate(BAUD_RATE_115200);
    serial->set_data_bits(DATA_BITS_8);
    serial->set_parity(PARITY_EVEN);
    serial->set_stop_bits(STOP_BITS_TWO);
    serial->set_flow_control(FLOW_CONTROL_HARDWARE);
    return config;
}

}   //namespace

// The mapping is only correct if the transfer layer accepts what it produces,
// so every check below parses the result instead of comparing strings.
TEST(ConfigMapping, TcpIpSettingsAreParsedByTheTransferLayer) {
    CollectorConfig config = MakeConfig();
    config.mutable_transfer()->mutable_tcp_ip()->set_host("127.0.0.1");
    config.mutable_transfer()->mutable_tcp_ip()->set_port(2323);

    app::Settings settings = config::ToTransferSettings(config);
    ASSERT_EQ(settings["type"], "TCP/IP");

    transfer::TcpIpSettings parsed = transfer::GetTcpIpSettingsFromMap(settings);
    EXPECT_EQ(parsed.host.to_string(), "127.0.0.1");
    EXPECT_EQ(parsed.port, 2323);
}

TEST(ConfigMapping, SerialSettingsAreParsedByTheTransferLayer) {
    app::Settings settings = config::ToTransferSettings(MakeSerialConfig());
    ASSERT_EQ(settings["type"], "Serial");

    transfer::SerialSettings parsed = transfer::GetSerialSettingsFromMap(settings);
    EXPECT_EQ(parsed.port_name, "/dev/ttyUSB0");
    EXPECT_EQ(parsed.baud_rate.value(), 115200u);
    EXPECT_EQ(parsed.data_bits.value(), 8u);
    EXPECT_EQ(parsed.parity.value(), asio::serial_port_base::parity::even);
    EXPECT_EQ(parsed.stop_bits.value(), asio::serial_port_base::stop_bits::two);
    EXPECT_EQ(parsed.flow_control.value(), asio::serial_port_base::flow_control::hardware);
}

// Walks the enums through their descriptors: a value added to the contract and
// forgotten here fails the test instead of the collector at runtime.
TEST(ConfigMapping, EveryValueOfEverySerialEnumIsMapped) {
    const google::protobuf::EnumDescriptor* enums[] = {
        BaudRate_descriptor(), DataBits_descriptor(), Parity_descriptor(),
        StopBits_descriptor(), FlowControl_descriptor()};

    for (const google::protobuf::EnumDescriptor* enum_type : enums) {
        for (int i = 0; i < enum_type->value_count(); ++i) {
            const int number = enum_type->value(i)->number();
            if (number == 0) {
                continue;   // UNSPECIFIED, rejected on purpose
            }

            CollectorConfig config = MakeSerialConfig();
            SerialTransfer* serial = config.mutable_transfer()->mutable_serial();
            if (enum_type == BaudRate_descriptor()) {
                serial->set_baud_rate(static_cast<BaudRate>(number));
            } else if (enum_type == DataBits_descriptor()) {
                serial->set_data_bits(static_cast<DataBits>(number));
            } else if (enum_type == Parity_descriptor()) {
                serial->set_parity(static_cast<Parity>(number));
            } else if (enum_type == StopBits_descriptor()) {
                serial->set_stop_bits(static_cast<StopBits>(number));
            } else {
                serial->set_flow_control(static_cast<FlowControl>(number));
            }

            // Space and mark parity have no portable counterpart: asio offers
            // the three modes every platform agrees on, so those two are
            // refused rather than silently turned into something else.
            const std::string name = enum_type->value(i)->name();
            if (name == "PARITY_SPACE" || name == "PARITY_MARK") {
                EXPECT_THROW(
                    transfer::GetSerialSettingsFromMap(config::ToTransferSettings(config)),
                    std::invalid_argument)
                    << name;
                continue;
            }

            EXPECT_NO_THROW(
                transfer::GetSerialSettingsFromMap(config::ToTransferSettings(config)))
                << enum_type->name() << " value " << name;
        }
    }
}

TEST(ConfigMapping, ConfigWithoutTransferIsRejected) {
    EXPECT_THROW(config::ToTransferSettings(MakeConfig()), std::invalid_argument);
}

TEST(ConfigMapping, UnspecifiedSerialFieldIsRejected) {
    CollectorConfig config = MakeSerialConfig();
    config.mutable_transfer()->mutable_serial()->set_parity(PARITY_UNSPECIFIED);

    EXPECT_THROW(config::ToTransferSettings(config), std::invalid_argument);
}

// The socket layer takes an address, not a name: a host that is not numeric has
// to be refused where the operator can see it.
TEST(ConfigMapping, HostThatIsNotAnAddressIsRejected) {
    CollectorConfig config = MakeConfig();
    config.mutable_transfer()->mutable_tcp_ip()->set_host("controller.local");
    config.mutable_transfer()->mutable_tcp_ip()->set_port(2323);

    EXPECT_THROW(transfer::GetTcpIpSettingsFromMap(config::ToTransferSettings(config)),
                 std::invalid_argument);
}

TEST(ConfigMapping, KafkaSettingsAreCarriedOver) {
    config::KafkaSettings settings = config::ToKafkaSettings(MakeConfig());
    EXPECT_EQ(settings.brokers, "kafka:9092");
    EXPECT_EQ(settings.topic, "sensor-data");
}

TEST(ConfigMapping, IncompleteKafkaSettingsAreRejected) {
    CollectorConfig config = MakeConfig();
    config.mutable_kafka()->clear_topic();

    EXPECT_THROW(config::ToKafkaSettings(config), std::invalid_argument);
}
