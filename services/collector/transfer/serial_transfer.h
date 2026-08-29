#ifndef SERIAL_TRANSFER_H
#define SERIAL_TRANSFER_H

#include <array>

#include <asio/io_context.hpp>
#include <asio/serial_port.hpp>

#include "json_framer.h"
#include "serial_transfer_domain.h"
#include "transfer_interface.h"

namespace transfer {

// Opens the port, applies the six parameters of the contract and reads what the
// controller writes into it.
class SerialTransfer : public TransferInterface {
public:
    SerialTransfer(const Settings& settings, asio::io_context& io);
    ~SerialTransfer() override;

    bool Start() override;
    void Stop() override;

private:
    SerialSettings settings_;
    asio::serial_port port_;
    JsonFramer framer_;
    std::array<char, 4096> chunk_{};

    void Read();
};

}   //transfer

#endif // SERIAL_TRANSFER_H
