#ifndef TRANSFER_FACTORY_H
#define TRANSFER_FACTORY_H

#include <memory>

#include "transfer_interface.h"
#include "serial_transfer.h"

namespace transfer {

class TransferFactory
{
public:
    static std::unique_ptr<TransferInterface> CreateTransfer(const std::string& type) {
        if (type == "serial") {
            return std::make_unique<SerialTransfer>();
        } else if (type == "tcpip") {
            //TODO
        }
        throw std::invalid_argument("Unknown transfer to create");
    }

    static std::unique_ptr<TransferInterface> CreateTransfer(const QHash<QString,QString>& settings) {
        if (settings["type"] == "serial") {
            return std::make_unique<SerialTransfer>(settings);
        } else if (settings["type"] == "tcpip") {
            //TODO
        }
        throw std::invalid_argument("Unknown transfer to create");
    }
};

}   //transfer

#endif // TRANSFER_FACTORY_H
