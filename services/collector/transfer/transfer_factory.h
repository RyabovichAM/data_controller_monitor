#ifndef TRANSFER_FACTORY_H
#define TRANSFER_FACTORY_H

#include <memory>

#include <asio/io_context.hpp>

#include "transfer_interface.h"

namespace transfer {

class TransferFactory {
public:
    // Throws std::invalid_argument on a type nobody can build or on settings
    // the transport would refuse — the caller keeps what it was running.
    static std::unique_ptr<TransferInterface> CreateTransfer(const Settings& settings,
                                                             asio::io_context& io);
};

}   //transfer

#endif // TRANSFER_FACTORY_H
