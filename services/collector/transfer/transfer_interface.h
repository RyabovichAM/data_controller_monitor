#ifndef TRANSFER_INTERFACE_H
#define TRANSFER_INTERFACE_H

#include <utility>

#include "transfer_domain.h"

namespace transfer {

// A source of JSON messages. The transports are asio objects underneath, so
// they need no loop of their own: they are given the io_context and report
// through these callbacks.
class TransferInterface {
public:
    virtual ~TransferInterface() = default;

    // False if the source could not be opened; the reason goes to the error
    // handler. Everything after that happens in the io_context.
    virtual bool Start() = 0;
    virtual void Stop() = 0;

    void SetJsonHandler(JsonHandler handler) {
        json_handler_ = std::move(handler);
    }

    void SetErrorHandler(ErrorHandler handler) {
        error_handler_ = std::move(handler);
    }

protected:
    const JsonHandler& json_handler() const {
        return json_handler_;
    }

    void ReportJson(const std::string& json) const {
        if (json_handler_) {
            json_handler_(json);
        }
    }

    void ReportError(const std::string& error) const {
        if (error_handler_) {
            error_handler_(error);
        }
    }

private:
    JsonHandler json_handler_{nullptr};
    ErrorHandler error_handler_{nullptr};
};

}   //transfer

#endif // TRANSFER_INTERFACE_H
