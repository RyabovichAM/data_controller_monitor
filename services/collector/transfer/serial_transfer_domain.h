#ifndef SERIAL_TRANSFER_DOMAIN_H
#define SERIAL_TRANSFER_DOMAIN_H

#include <string>

#include <asio/serial_port_base.hpp>

#include "transfer_domain.h"

namespace transfer {

// The six parameters of the contract, in the form asio takes them. On Linux
// they become termios flags, on Windows a DCB — which is the point of going
// through asio rather than termios directly.
struct SerialSettings {
    std::string port_name;
    asio::serial_port_base::baud_rate baud_rate{9600};
    asio::serial_port_base::character_size data_bits{8};
    asio::serial_port_base::parity parity{asio::serial_port_base::parity::none};
    asio::serial_port_base::stop_bits stop_bits{asio::serial_port_base::stop_bits::one};
    asio::serial_port_base::flow_control flow_control{
        asio::serial_port_base::flow_control::none};
};

// The strings are the ones the contract is mapped into — "115200", "None",
// "1.5" and so on. Anything else throws std::invalid_argument.
SerialSettings GetSerialSettingsFromMap(const Settings& settings);

}   //transfer

#endif // SERIAL_TRANSFER_DOMAIN_H
