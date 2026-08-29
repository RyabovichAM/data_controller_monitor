#ifndef TRANSFER_FACTORY_H
#define TRANSFER_FACTORY_H

#include <memory>

#include "transfer_interface.h"
#include "tcpip_transfer.h"
#include "serial_transfer.h"

namespace transfer {

class TransferFactory
{
public:
    static std::unique_ptr<TransferInterface> CreateTransfer(const std::string& type) {
        if (type == "Serial") {
            return std::make_unique<SerialTransfer>();
        } else if (type == "TCP/IP") {
            return std::make_unique<TcpIpTransfer>();
        }
        Q_ASSERT("TransferFactory: Unknown transfer to create");
        return nullptr;
    }

    static std::unique_ptr<TransferInterface> CreateTransfer(const QHash<QString,QString>& settings) {
        if (settings["type"] == "Serial") {
            return std::make_unique<SerialTransfer>(settings);
        } else if (settings["type"] == "TCP/IP") {
            return std::make_unique<TcpIpTransfer>(settings);
        }
        Q_ASSERT("TransferFactory: Unknown transfer to create");
        return nullptr;
    }
};

}   //transfer

#endif // TRANSFER_FACTORY_H
