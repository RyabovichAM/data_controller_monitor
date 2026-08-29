#ifndef MONITOR_UNIT_H
#define MONITOR_UNIT_H

#include <functional>
#include <memory>
#include <string>

#include <asio/io_context.hpp>

#include "app_domain.h"
#include "transfer_interface.h"

namespace app {

// One collector: a transport underneath, a handler above.
//
// The observer used to be a QObject with a virtual Update; a std::function
// says the same thing, and main.cpp no longer needs a class to hold one line.
class MonitorUnit {
public:
    using DataHandler = std::function<void(const std::string& json)>;
    using ErrorHandler = std::function<void(const std::string& error)>;

    explicit MonitorUnit(asio::io_context& io);
    ~MonitorUnit();

    void SetName(std::string name);
    const std::string& Name() const;

    void SetSettings(const Settings& settings);
    const Settings& CurrentSettings() const;

    void SetDataHandler(DataHandler handler);
    void SetErrorHandler(ErrorHandler handler);

    // Builds the transport for the current settings and starts reading. False
    // if the settings are unusable or the source refused to open.
    bool Start();
    void Stop();

private:
    asio::io_context& io_;
    Settings settings_;
    std::unique_ptr<transfer::TransferInterface> transfer_;

    std::string name_;
    DataHandler data_handler_{nullptr};
    ErrorHandler error_handler_{nullptr};

    void OnJson(const std::string& json);
    void ReportError(const std::string& error) const;
};

}   //app

#endif // MONITOR_UNIT_H
