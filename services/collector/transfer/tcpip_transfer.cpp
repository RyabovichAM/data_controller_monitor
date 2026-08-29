#include "tcpip_transfer.h"

#include <array>
#include <system_error>
#include <utility>

#include "json_framer.h"

namespace transfer {

namespace {

constexpr size_t kReadChunk = 4096;

// One connected controller. It keeps itself alive for as long as a read is in
// flight — the asio way: the handler holds a shared_ptr to its own connection,
// and the last handler to run lets it go.
class Connection : public std::enable_shared_from_this<Connection> {
public:
    Connection(asio::ip::tcp::socket socket, JsonHandler json_handler,
               ErrorHandler error_handler)
        : socket_{std::move(socket)}
        , error_handler_{std::move(error_handler)}
        , framer_{std::move(json_handler)} {
    }

    void Read() {
        auto self = shared_from_this();
        socket_.async_read_some(
            asio::buffer(chunk_), [this, self](std::error_code error, size_t size) {
                if (error) {
                    // End of file is the controller hanging up, not a fault.
                    if (error != asio::error::eof && error != asio::error::operation_aborted &&
                        error_handler_) {
                        error_handler_("connection: " + error.message());
                    }
                    return;
                }

                framer_.Feed(chunk_.data(), size);
                Read();
            });
    }

private:
    asio::ip::tcp::socket socket_;
    ErrorHandler error_handler_;
    JsonFramer framer_;
    std::array<char, kReadChunk> chunk_{};
};

}   //namespace

TcpIpTransfer::TcpIpTransfer(const Settings& settings, asio::io_context& io)
    : settings_{GetTcpIpSettingsFromMap(settings)}
    , io_{io} {
}

TcpIpTransfer::~TcpIpTransfer() {
    Stop();
}

bool TcpIpTransfer::Start() {
    const asio::ip::tcp::endpoint endpoint{settings_.host, settings_.port};

    try {
        acceptor_.emplace(io_);
        acceptor_->open(endpoint.protocol());
        // Without this a restart within the TIME_WAIT window cannot take its
        // own port back, and the collector would refuse to start after a
        // reconfigure.
        acceptor_->set_option(asio::ip::tcp::acceptor::reuse_address{true});
        acceptor_->bind(endpoint);
        acceptor_->listen();
    } catch (const std::system_error& error) {
        ReportError("listen " + endpoint.address().to_string() + ":" +
                    std::to_string(endpoint.port()) + ": " + error.code().message());
        acceptor_.reset();
        return false;
    }

    Accept();
    return true;
}

void TcpIpTransfer::Stop() {
    if (acceptor_) {
        std::error_code ignored;
        acceptor_->close(ignored);
        acceptor_.reset();
    }
    // The connections close themselves: their reads end with operation_aborted
    // and the last handler drops the final reference.
}

void TcpIpTransfer::Accept() {
    if (!acceptor_) {
        return;
    }

    acceptor_->async_accept([this](std::error_code error, asio::ip::tcp::socket socket) {
        if (error) {
            if (error != asio::error::operation_aborted) {
                ReportError("accept: " + error.message());
            }
            return;   // the acceptor is closed, nothing left to wait for
        }

        // A framer per connection: two controllers must not have their messages
        // interleaved into one another's.
        std::make_shared<Connection>(
            std::move(socket), [this](const std::string& json) { ReportJson(json); },
            [this](const std::string& message) { ReportError(message); })
            ->Read();

        Accept();
    });
}

}   //transfer
