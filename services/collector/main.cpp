#include <cstdio>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <string>

#include <asio/io_context.hpp>
#include <asio/post.hpp>
#include <asio/signal_set.hpp>

#include "app/monitor_unit.h"
#include "config/config_client.h"
#include "config/config_mapping.h"
#include "kafka_producer.h"

namespace {

std::string Env(const char* name, const std::string& fallback) {
    const char* value = std::getenv(name);
    return (value && *value) ? std::string{value} : fallback;
}

// Everything a configuration drives. Lives in the loop thread and is only ever
// touched from it: configs arrive on the gRPC client's thread and are handed
// over through EventLoop::Post.
class Runtime {
public:
    Runtime(asio::io_context& io, std::string collector_id)
        : unit_{io}
        , collector_id_{std::move(collector_id)} {
        unit_.SetName(collector_id_);
        unit_.SetErrorHandler([](const std::string& error) {
            std::cout << "[transfer] " << error << std::endl;
        });
        unit_.SetDataHandler([this](const std::string& json) {
            if (producer_) {
                producer_->Publish(collector_id_, json);
            }
        });
    }

    void Apply(const dcm::config::v1::CollectorConfig& config);
    void Shutdown();

private:
    app::MonitorUnit unit_;
    std::string collector_id_;

    std::unique_ptr<kafka::KafkaProducer> producer_;
    config::KafkaSettings kafka_settings_;
    app::Settings transfer_settings_;
    bool running_{false};

    void RebuildProducer(const config::KafkaSettings& settings);
};

void Runtime::Apply(const dcm::config::v1::CollectorConfig& config) {
    app::Settings transfer_settings;
    config::KafkaSettings kafka_settings;
    try {
        transfer_settings = config::ToTransferSettings(config);
        kafka_settings = config::ToKafkaSettings(config);
    } catch (const std::exception& error) {
        // Nothing has been touched yet, so the collector keeps running on what
        // it already had.
        std::cout << "[collector] version " << config.version() << " ignored: " << error.what()
                  << std::endl;
        return;
    }

    if (!producer_ || kafka_settings != kafka_settings_) {
        RebuildProducer(kafka_settings);
    }

    // Reopening a listening socket drops whatever is connected to it, so an
    // unchanged transport is left alone.
    if (running_ && transfer_settings == transfer_settings_) {
        std::cout << "[collector] version " << config.version()
                  << " applied, transport unchanged" << std::endl;
        return;
    }

    unit_.Stop();
    unit_.SetSettings(transfer_settings);
    transfer_settings_ = transfer_settings;

    running_ = unit_.Start();
    std::cout << "[collector] version " << config.version()
              << (running_ ? " applied: " : " failed to start: ")
              << transfer_settings["type"] << std::endl;
}

void Runtime::Shutdown() {
    unit_.Stop();
    running_ = false;

    if (producer_) {
        producer_->Flush();
    }
}

void Runtime::RebuildProducer(const config::KafkaSettings& settings) {
    if (producer_) {
        // Whatever was collected before the switch still belongs in the old topic.
        producer_->Flush();
    }

    producer_ = std::make_unique<kafka::KafkaProducer>(settings.brokers, settings.topic);
    producer_->SetErrorHandler([](const std::string& error) {
        std::cout << "[kafka] " << error << std::endl;
    });

    kafka_settings_ = settings;

    std::cout << "[collector] kafka " << settings.brokers << " topic=" << settings.topic
              << std::endl;
}

}   //namespace

int main() {
    // Container stdout is a pipe, so it is block buffered by default and the
    // log of a service that never exits would sit in memory.
    setvbuf(stdout, nullptr, _IOLBF, 0);

    const std::string collector_id = Env("COLLECTOR_ID", "collector-1");
    const std::string config_address = Env("CONFIG_SERVICE_ADDRESS", "config-service:50051");

    asio::io_context io;
    Runtime runtime{io, collector_id};

    // asio's own signal handling: the handler runs in the io_context like any
    // other work, so there is nothing to make async-signal-safe.
    asio::signal_set signals{io, SIGINT, SIGTERM};
    signals.async_wait([&io](const std::error_code&, int) { io.stop(); });

    config::ConfigClient client{config_address, collector_id};
    client.SetLogHandler([](const std::string& message) {
        std::cout << "[config] " << message << std::endl;
    });
    client.SetConfigHandler([&io, &runtime](const dcm::config::v1::CollectorConfig& config) {
        // Arrives on the client's thread; the transports belong to the
        // io_context, so the work is handed over rather than done here.
        asio::post(io, [&runtime, config]() { runtime.Apply(config); });
    });

    std::cout << "[collector] " << collector_id << " -> config-service " << config_address
              << std::endl;

    client.Start();
    io.run();

    client.Stop();
    runtime.Shutdown();

    return 0;
}
