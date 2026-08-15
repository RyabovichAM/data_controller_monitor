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

#include "data_storage/storage_registry.h"
#include "kafka/kafka_consumer.h"
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

}   //namespace

int main() {
    const std::string brokers = Env("KAFKA_BROKERS", "kafka:9092");
    const std::string topic = Env("KAFKA_TOPIC", "sensor-data");
    const std::string group_id = Env("KAFKA_GROUP_ID", "storage-service");
    const std::string root = Env("STORAGE_ROOT", "/data");
    const std::string data_format = Env("STORAGE_FORMAT", "text");
    const std::string period = Env("STORAGE_PERIOD_MS", "0");
    const std::string address = Env("STORAGE_SERVICE_ADDRESS", "0.0.0.0:50052");

    data_storage::StorageRegistry registry{root, data_format, period};

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

    std::cout << "[storage-service] " << brokers << " topic=" << topic << " root=" << root
              << " grpc=" << address << std::endl;

    consumer.Run();

    // The deadline cancels DataLoad streams, which would otherwise hold the
    // shutdown for as long as their clients keep reading.
    server->Shutdown(std::chrono::system_clock::now() + std::chrono::seconds{2});
    server_thread.join();

    return 0;
}
