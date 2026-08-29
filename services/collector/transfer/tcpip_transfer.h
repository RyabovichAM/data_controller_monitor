#ifndef TCPIP_TRANSFER_H
#define TCPIP_TRANSFER_H

#include <memory>
#include <optional>

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>

#include "tcpip_transfer_domain.h"
#include "transfer_interface.h"

namespace transfer {

// Listens for controllers rather than dialling out: they connect, it reads.
// Several at once are fine — every connection gets its own framer, because a
// message may arrive split across reads.
class TcpIpTransfer : public TransferInterface {
public:
    TcpIpTransfer(const Settings& settings, asio::io_context& io);
    ~TcpIpTransfer() override;

    bool Start() override;
    void Stop() override;

private:
    TcpIpSettings settings_;
    asio::io_context& io_;
    std::optional<asio::ip::tcp::acceptor> acceptor_;

    void Accept();
};

}   //transfer

#endif // TCPIP_TRANSFER_H
