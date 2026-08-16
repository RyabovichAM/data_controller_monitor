#include <atomic>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <thread>

#include <drogon/drogon.h>
#include <json/json.h>
#include <trantor/net/EventLoopThreadPool.h>

#include "api/api_handlers.h"
#include "clients/storage_client.h"
#include "kafka_consumer.h"
#include "realtime/realtime_hub.h"

namespace {

std::atomic_bool stop_requested{false};

void RequestStop(int /*signal*/) {
    stop_requested = true;
}

std::string Env(const char* name, const std::string& fallback) {
    const char* value = std::getenv(name);
    return (value && *value) ? std::string{value} : fallback;
}

// The controller sends whatever it likes; the browser gets it wrapped, with the
// payload as a JSON value rather than a string so the UI parses nothing twice.
// A message that is not JSON at all is dropped here instead of in every client.
bool BuildMessage(const std::string& collector_id, const std::string& payload,
                  std::string& message) {
    Json::CharReaderBuilder reader_builder;
    std::unique_ptr<Json::CharReader> reader{reader_builder.newCharReader()};

    Json::Value payload_json;
    std::string errors;
    if (!reader->parse(payload.data(), payload.data() + payload.size(), &payload_json,
                       &errors)) {
        return false;
    }

    Json::Value envelope;
    envelope["collector_id"] = collector_id;
    envelope["payload"] = payload_json;

    Json::StreamWriterBuilder writer_builder;
    writer_builder["indentation"] = "";
    message = Json::writeString(writer_builder, envelope);

    return true;
}

}   //namespace

int main() {
    // Trantor flushes its log only on a fatal message, and stdout of a
    // container is a pipe — block buffered. Without this the log of a service
    // that never exits stays in memory.
    setvbuf(stdout, nullptr, _IOLBF, 0);

    const std::string brokers = Env("KAFKA_BROKERS", "kafka:9092");
    const std::string topic = Env("KAFKA_TOPIC", "sensor-data");
    // A group of its own: the backend and storage-service both read every
    // message, they do not share the work.
    const std::string group_id = Env("KAFKA_GROUP_ID", "backend");
    const std::string storage_address = Env("STORAGE_SERVICE_ADDRESS", "storage-service:50052");
    const std::string listen_host = Env("BACKEND_HOST", "0.0.0.0");
    const uint16_t listen_port = static_cast<uint16_t>(std::stoi(Env("BACKEND_PORT", "8080")));

    clients::StorageClient storage{storage_address};

    // Two threads for the synchronous stubs. They are event loops only because
    // trantor already has a pool of them — no work of their own runs here.
    trantor::EventLoopThreadPool blocking_pool{2, "grpc"};
    blocking_pool.start();

    api::RegisterHandlers(storage, blocking_pool);

    kafka::KafkaConsumer consumer{brokers, topic, group_id};
    consumer.SetErrorHandler([](const std::string& error) {
        LOG_WARN << "[kafka] " << error;
    });
    consumer.SetMessageHandler([](const std::string& key, const std::string& payload) {
        std::string message;
        if (!BuildMessage(key, payload, message)) {
            LOG_WARN << "[kafka] " << key << " sent something that is not JSON";
            return;
        }

        realtime::Hub().Broadcast(key, message);
    });

    std::thread consumer_thread{[&consumer]() { consumer.Run(); }};

    std::signal(SIGINT, RequestStop);
    std::signal(SIGTERM, RequestStop);

    // A signal handler may only touch the flag; leaving the loop is the loop's
    // own business.
    drogon::app().getLoop()->runEvery(0.2, []() {
        if (stop_requested) {
            drogon::app().quit();
        }
    });

    LOG_INFO << "[backend] " << listen_host << ":" << listen_port << " kafka=" << brokers
             << " topic=" << topic << " storage=" << storage_address;

    drogon::app()
        .addListener(listen_host, listen_port)
        .setThreadNum(2)
        .setLogLevel(trantor::Logger::kInfo)
        .run();

    consumer.Stop();
    consumer_thread.join();

    return 0;
}
