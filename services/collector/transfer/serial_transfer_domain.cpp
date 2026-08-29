#include "serial_transfer_domain.h"

#include <stdexcept>

namespace transfer {

namespace {

using Base = asio::serial_port_base;

unsigned int GetBaudRateFromString(const std::string& text) {
    if (text == "1200" || text == "2400" || text == "4800" || text == "9600" ||
        text == "19200" || text == "38400" || text == "57600" || text == "115200") {
        return static_cast<unsigned int>(std::stoul(text));
    }

    throw std::invalid_argument("transfer: '" + text + "' is not a baud rate");
}

unsigned int GetDataBitsFromString(const std::string& text) {
    if (text == "5" || text == "6" || text == "7" || text == "8") {
        return static_cast<unsigned int>(std::stoul(text));
    }

    throw std::invalid_argument("transfer: '" + text + "' is not a data bits count");
}

Base::parity::type GetParityFromString(const std::string& text) {
    if (text == "None") return Base::parity::none;
    if (text == "Even") return Base::parity::even;
    if (text == "Odd") return Base::parity::odd;
    // Space and mark parity are not portable — asio offers the three every
    // platform agrees on, and the contract's other two have no counterpart.
    throw std::invalid_argument("transfer: '" + text + "' is not a portable parity");
}

Base::stop_bits::type GetStopBitsFromString(const std::string& text) {
    if (text == "1") return Base::stop_bits::one;
    if (text == "1.5") return Base::stop_bits::onepointfive;
    if (text == "2") return Base::stop_bits::two;

    throw std::invalid_argument("transfer: '" + text + "' is not a stop bits count");
}

Base::flow_control::type GetFlowControlFromString(const std::string& text) {
    if (text == "None") return Base::flow_control::none;
    if (text == "Hardware") return Base::flow_control::hardware;
    if (text == "Software") return Base::flow_control::software;

    throw std::invalid_argument("transfer: '" + text + "' is not a flow control");
}

}   //namespace

SerialSettings GetSerialSettingsFromMap(const Settings& settings) {
    SerialSettings result;

    result.port_name = Value(settings, "port_name");
    if (result.port_name.empty()) {
        throw std::invalid_argument("transfer: port_name is empty");
    }

    result.baud_rate = Base::baud_rate{GetBaudRateFromString(Value(settings, "baud_rate"))};
    result.data_bits =
        Base::character_size{GetDataBitsFromString(Value(settings, "data_bits"))};
    result.parity = Base::parity{GetParityFromString(Value(settings, "parity"))};
    result.stop_bits = Base::stop_bits{GetStopBitsFromString(Value(settings, "stop_bits"))};
    result.flow_control =
        Base::flow_control{GetFlowControlFromString(Value(settings, "flow_control"))};

    return result;
}

}   //transfer
