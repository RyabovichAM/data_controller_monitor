#ifndef CONFIG_CLIENT_H
#define CONFIG_CLIENT_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <grpcpp/grpcpp.h>

#include "config_service.grpc.pb.h"

namespace config {

using ConfigHandler = std::function<void(const dcm::config::v1::CollectorConfig&)>;
using LogHandler = std::function<void(const std::string&)>;

// Keeps the collector's configuration up to date: one GetConfig on start, then
// a WatchConfig stream that stays open for as long as the process lives.
//
// Runs in a thread of its own — the stream call blocks — so the handler is
// invoked from that thread, not from the one owning the transports.
class ConfigClient {
public:
    ConfigClient(const std::string& address, const std::string& collector_id);
    ~ConfigClient();

    void SetConfigHandler(ConfigHandler handler);
    void SetLogHandler(LogHandler handler);

    void Start();
    void Stop();

private:
    std::string collector_id_;
    std::unique_ptr<dcm::config::v1::ConfigService::Stub> stub_;

    ConfigHandler config_handler_{nullptr};
    LogHandler log_handler_{nullptr};

    std::thread worker_;
    std::atomic_bool stopped_{true};

    // The call in flight, so Stop() can cut it short instead of waiting for the
    // server to say something.
    std::mutex context_mutex_;
    grpc::ClientContext* active_context_{nullptr};

    // Last version handed to the handler. Sent as known_version so a reopened
    // stream does not replay a config the collector is already running.
    int64_t known_version_{0};

    void Run();
    void FetchOnce();
    void Watch();

    void Deliver(const dcm::config::v1::CollectorConfig& config);
    void Log(const std::string& message);

    void SetActiveContext(grpc::ClientContext* context);
    void ClearActiveContext();
};

}   //config

#endif // CONFIG_CLIENT_H
