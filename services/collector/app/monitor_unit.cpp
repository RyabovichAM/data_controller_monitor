#include "monitor_unit.h"

#include <exception>
#include <utility>

#include <nlohmann/json.hpp>

#include "transfer_factory.h"

namespace app {

MonitorUnit::MonitorUnit(asio::io_context& io)
    : io_{io} {
}

MonitorUnit::~MonitorUnit() {
    Stop();
}

void MonitorUnit::SetName(std::string name) {
    name_ = std::move(name);
}

const std::string& MonitorUnit::Name() const {
    return name_;
}

void MonitorUnit::SetSettings(const Settings& settings) {
    settings_ = settings;
}

const Settings& MonitorUnit::CurrentSettings() const {
    return settings_;
}

void MonitorUnit::SetDataHandler(DataHandler handler) {
    data_handler_ = std::move(handler);
}

void MonitorUnit::SetErrorHandler(ErrorHandler handler) {
    error_handler_ = std::move(handler);
}

bool MonitorUnit::Start() {
    try {
        transfer_ = transfer::TransferFactory::CreateTransfer(settings_, io_);
    } catch (const std::exception& error) {
        ReportError(error.what());
        return false;
    }

    transfer_->SetJsonHandler([this](const std::string& json) { OnJson(json); });
    transfer_->SetErrorHandler([this](const std::string& error) { ReportError(error); });

    if (!transfer_->Start()) {
        transfer_.reset();
        return false;
    }

    return true;
}

void MonitorUnit::Stop() {
    if (transfer_) {
        transfer_->Stop();
        transfer_.reset();
    }
}

void MonitorUnit::OnJson(const std::string& json) {
    // The framer only guarantees balanced braces, so the message is parsed
    // here — and published in its compact form. That is not cosmetic: storage
    // writes a sample per line, and a payload with a line break inside would
    // split the record in two.
    nlohmann::json parsed = nlohmann::json::parse(json, nullptr, false);
    if (parsed.is_discarded()) {
        ReportError("received something that is not JSON, dropped");
        return;
    }

    if (data_handler_) {
        data_handler_(parsed.dump());
    }
}

void MonitorUnit::ReportError(const std::string& error) const {
    if (error_handler_) {
        error_handler_(error);
    }
}

}   //app
