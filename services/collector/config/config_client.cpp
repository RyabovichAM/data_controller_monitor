#include "config_client.h"

#include <chrono>

namespace config {

namespace {

using namespace dcm::config::v1;

// The service may be starting, restarting or simply unreachable for a while;
// there is nothing better to do than to come back later.
constexpr std::chrono::seconds kRetryDelay{2};
constexpr std::chrono::seconds kUnaryDeadline{5};

}   //namespace

ConfigClient::ConfigClient(const std::string& address, const std::string& collector_id)
    : collector_id_{collector_id}
    , stub_{ConfigService::NewStub(
          grpc::CreateChannel(address, grpc::InsecureChannelCredentials()))} {
}

ConfigClient::~ConfigClient() {
    Stop();
}

void ConfigClient::SetConfigHandler(ConfigHandler handler) {
    config_handler_ = handler;
}

void ConfigClient::SetLogHandler(LogHandler handler) {
    log_handler_ = handler;
}

void ConfigClient::Start() {
    if (worker_.joinable()) {
        return;
    }

    stopped_ = false;
    worker_ = std::thread{[this]() { Run(); }};
}

void ConfigClient::Stop() {
    if (!worker_.joinable()) {
        return;
    }

    stopped_ = true;
    {
        std::lock_guard<std::mutex> lock{context_mutex_};
        if (active_context_) {
            active_context_->TryCancel();
        }
    }

    worker_.join();
}

void ConfigClient::Run() {
    while (!stopped_) {
        // Only on the very first pass: once something has been applied, a
        // reopened WatchConfig delivers everything newer by itself.
        if (known_version_ == 0) {
            FetchOnce();
        }

        Watch();

        for (int slept = 0; !stopped_ && slept < kRetryDelay.count(); ++slept) {
            std::this_thread::sleep_for(std::chrono::seconds{1});
        }
    }
}

void ConfigClient::FetchOnce() {
    GetConfigRequest request;
    request.set_collector_id(collector_id_);

    CollectorConfig config;
    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + kUnaryDeadline);
    SetActiveContext(&context);

    grpc::Status status = stub_->GetConfig(&context, request, &config);
    ClearActiveContext();

    if (status.ok()) {
        Deliver(config);
        return;
    }

    if (status.error_code() == grpc::StatusCode::NOT_FOUND) {
        // Normal for a collector nobody has configured yet: the watch below
        // will pick the config up as soon as it appears.
        Log("no config yet, waiting for one");
        return;
    }

    if (!stopped_) {
        Log("GetConfig failed: " + status.error_message());
    }
}

void ConfigClient::Watch() {
    WatchConfigRequest request;
    request.set_collector_id(collector_id_);
    request.set_known_version(known_version_);

    grpc::ClientContext context;
    SetActiveContext(&context);

    std::unique_ptr<grpc::ClientReader<CollectorConfig>> reader =
        stub_->WatchConfig(&context, request);

    CollectorConfig config;
    while (reader->Read(&config)) {
        Deliver(config);
    }

    grpc::Status status = reader->Finish();
    ClearActiveContext();

    if (!stopped_ && !status.ok()) {
        Log("watch ended: " + status.error_message());
    }
}

void ConfigClient::Deliver(const CollectorConfig& config) {
    known_version_ = config.version();

    if (config_handler_) {
        config_handler_(config);
    }
}

void ConfigClient::Log(const std::string& message) {
    if (log_handler_) {
        log_handler_(message);
    }
}

void ConfigClient::SetActiveContext(grpc::ClientContext* context) {
    std::lock_guard<std::mutex> lock{context_mutex_};
    active_context_ = context;

    // Stop() may have run between creating the context and getting here, and
    // then there would be nobody left to cancel this call.
    if (stopped_) {
        context->TryCancel();
    }
}

void ConfigClient::ClearActiveContext() {
    std::lock_guard<std::mutex> lock{context_mutex_};
    active_context_ = nullptr;
}

}   //config
