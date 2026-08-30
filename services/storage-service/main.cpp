#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpcpp/grpcpp.h>

#include "data_storage/data_storage_domain.h"
#include "data_storage/storage_registry.h"
#include "kafka_consumer.h"
#include "service/storage_service_impl.h"

namespace {

kafka::KafkaConsumer* running_consumer{nullptr};

void RequestStop(int /*signal*/) {
    if (running_consumer) {
        // Only sets a flag the polling loop reads.
        running_consumer->Stop();
    }
}

std::string Env(const char* name, const std::string& fallback) {
    const char* value = std::getenv(name);
    return (value && *value) ? std::string{value} : fallback;
}

// Everything StorageRegistry needs to build a storage or a directory for any
// collector, gathered from the environment once at start-up. "timescaledb" is
// the default: config-service already made a proper database the operating
// choice everywhere else, and "file" stays a fully supported, tested
// alternative for a deployment that would rather not run Postgres.
data_storage::Settings BuildStorageSettings() {
    data_storage::Settings settings;
    settings["type"] = Env("STORAGE_TYPE", "timescaledb");
    settings["period"] = Env("STORAGE_PERIOD_MS", "0");

    if (settings["type"] == "file") {
        settings["root"] = Env("STORAGE_ROOT", "/data");
        settings["data_format"] = Env("STORAGE_FORMAT", "text");
    } else {
        settings["dsn"] =
            Env("POSTGRES_DSN", "postgresql://dcm:dcm@timescaledb:5432/dcm_history");
    }

    return settings;
}

}   //namespace

int main() {
    const std::string brokers = Env("KAFKA_BROKERS", "kafka:9092");
    const std::string topic = Env("KAFKA_TOPIC", "sensor-data");
    const std::string group_id = Env("KAFKA_GROUP_ID", "storage-service");
    const std::string address = Env("STORAGE_SERVICE_ADDRESS", "0.0.0.0:50052");
    const data_storage::Settings storage_settings = BuildStorageSettings();

    data_storage::StorageRegistry registry{storage_settings};

    kafka::KafkaConsumer consumer{brokers, topic, group_id};
    consumer.SetErrorHandler([](const std::string& error) {
        std::cout << "[kafka] " << error << std::endl;
    });
    consumer.SetMessageHandler([&registry](const std::string& key, const std::string& payload) {
        try {
            registry.ForCollector(key)->DataSave(payload);
        } catch (const std::exception& error) {
            // A storage that cannot be built must not take the whole consumer
            // down: the next message may well be from another collector.
            std::cout << "[storage:" << key << "] " << error.what() << std::endl;
        }
    });

    storage::StorageServiceImpl service{registry};

    grpc::ServerBuilder builder;
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server{builder.BuildAndStart()};
    if (!server) {
        std::cerr << "[storage-service] cannot listen on " << address << std::endl;
        return 1;
    }

    // The consumer keeps the main thread, the server gets one of its own: both
    // block, and the consumer is the one that has to see the signal.
    std::thread server_thread{[&server]() { server->Wait(); }};

    running_consumer = &consumer;
    std::signal(SIGINT, RequestStop);
    std::signal(SIGTERM, RequestStop);

    std::cout << "[storage-service] " << brokers << " topic=" << topic << " storage="
              << storage_settings.at("type") << " grpc=" << address << std::endl;

    consumer.Run();

    // The deadline cancels DataLoad streams, which would otherwise hold the
    // shutdown for as long as their clients keep reading.
    server->Shutdown(std::chrono::system_clock::now() + std::chrono::seconds{2});
    server_thread.join();

    return 0;
}
