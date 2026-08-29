#include "transfer_factory.h"

#include <stdexcept>

#include "serial_transfer.h"
#include "tcpip_transfer.h"

namespace transfer {

std::unique_ptr<TransferInterface> TransferFactory::CreateTransfer(const Settings& settings,
                                                                   asio::io_context& io) {
    const std::string type = Value(settings, "type");

    if (type == "TCP/IP") {
        return std::make_unique<TcpIpTransfer>(settings, io);
    }
    if (type == "Serial") {
        return std::make_unique<SerialTransfer>(settings, io);
    }

    throw std::invalid_argument("transfer: unknown transport '" + type + "'");
}

}   //transfer
