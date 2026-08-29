#ifndef TCPIP_TRANSFER_DOMAIN_H
#define TCPIP_TRANSFER_DOMAIN_H

#include <cstdint>

#include <asio/ip/address.hpp>

#include "transfer_domain.h"

namespace transfer {

struct TcpIpSettings {
    asio::ip::address host{asio::ip::address_v4::any()};
    uint16_t port{0};
};

// Throws std::invalid_argument on an address or a port a listening socket
// cannot be given — the caller keeps running on its previous configuration.
TcpIpSettings GetTcpIpSettingsFromMap(const Settings& settings);

}   //transfer

#endif // TCPIP_TRANSFER_DOMAIN_H
