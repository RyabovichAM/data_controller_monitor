#include "serial_transfer.h"

#include <system_error>

namespace transfer {

SerialTransfer::SerialTransfer(const Settings& settings, asio::io_context& io)
    : settings_{GetSerialSettingsFromMap(settings)}
    , port_{io}
    , framer_{[this](const std::string& json) { ReportJson(json); }} {
}

SerialTransfer::~SerialTransfer() {
    Stop();
}

bool SerialTransfer::Start() {
    try {
        port_.open(settings_.port_name);

        port_.set_option(settings_.baud_rate);
        port_.set_option(settings_.data_bits);
        port_.set_option(settings_.parity);
        port_.set_option(settings_.stop_bits);
        port_.set_option(settings_.flow_control);
    } catch (const std::system_error& error) {
        ReportError("open " + settings_.port_name + ": " + error.code().message());
        Stop();
        return false;
    }

    framer_.Reset();
    Read();

    return true;
}

void SerialTransfer::Stop() {
    if (port_.is_open()) {
        std::error_code ignored;
        port_.close(ignored);
    }
}

void SerialTransfer::Read() {
    port_.async_read_some(asio::buffer(chunk_), [this](std::error_code error, size_t size) {
        if (error) {
            if (error != asio::error::operation_aborted) {
                ReportError("read " + settings_.port_name + ": " + error.message());
            }
            return;
        }

        framer_.Feed(chunk_.data(), size);
        Read();
    });
}

}   //transfer
