#include "tcpip_transfer_domain.h"

#include <stdexcept>
#include <system_error>

namespace transfer {

TcpIpSettings GetTcpIpSettingsFromMap(const Settings& settings) {
    TcpIpSettings result;

    const std::string host = Value(settings, "host");
    if (host.empty()) {
        throw std::invalid_argument("transfer: host is empty");
    }

    // make_address parses, it does not resolve: a listening socket has nothing
    // to look up, and a name here would be a mistake worth reporting.
    std::error_code error;
    result.host = asio::ip::make_address(host, error);
    if (error) {
        throw std::invalid_argument("transfer: '" + host + "' is not an address");
    }

    const std::string port = Value(settings, "port");
    if (port.empty()) {
        throw std::invalid_argument("transfer: port is empty");
    }

    try {
        const long number = std::stol(port);
        if (number <= 0 || number > 65535) {
            throw std::out_of_range{port};
        }
        result.port = static_cast<uint16_t>(number);
    } catch (const std::exception&) {
        throw std::invalid_argument("transfer: '" + port + "' is not a port");
    }

    return result;
}

}   //transfer
