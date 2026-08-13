#include <atomic>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include "data_storage/data_storage_factory.h"
#include "kafka/kafka_consumer.h"

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

// One storage per collector, created when its first message arrives — the
// service learns about collectors from the topic, not from a configuration.
class StorageRegistry {
public:
    StorageRegistry(const std::string& root, const std::string& data_format,
                    const std::string& period)
        : root_{root}
        , data_format_{data_format}
        , period_{period} {
    }

    data_storage::DataStorageInterface* ForCollector(const std::string& collector_id) {
        auto it = storages_.find(collector_id);
        if (it != storages_.end()) {
            return it->second.get();
        }

        const std::string location = root_ + "/" + collector_id + "/";
        std::filesystem::create_directories(location);

        data_storage::Settings settings;
        settings["type"] = "file";
        settings["location"] = location;
        settings["data_format"] = data_format_;
        settings["period"] = period_;

        auto storage = data_storage::DataStorageFactory::CreateDataStorage(settings);
        storage->SetErrorHandler([collector_id](const std::string& error) {
            std::cout << "[storage:" << collector_id << "] " << error << std::endl;
        });
        storage->Open();

        return storages_.emplace(collector_id, std::move(storage)).first->second.get();
    }

private:
    std::unordered_map<std::string, std::unique_ptr<data_storage::DataStorageInterface>> storages_;
    std::string root_;
    std::string data_format_;
    std::string period_;
};

}   //namespace

int main() {
    const std::string brokers = Env("KAFKA_BROKERS", "kafka:9092");
    const std::string topic = Env("KAFKA_TOPIC", "sensor-data");
    const std::string group_id = Env("KAFKA_GROUP_ID", "storage-service");
    const std::string root = Env("STORAGE_ROOT", "/data");
    const std::string data_format = Env("STORAGE_FORMAT", "text");
    const std::string period = Env("STORAGE_PERIOD_MS", "0");

    StorageRegistry registry{root, data_format, period};

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

    running_consumer = &consumer;
    std::signal(SIGINT, RequestStop);
    std::signal(SIGTERM, RequestStop);

    std::cout << "[storage-service] " << brokers << " topic=" << topic << " root=" << root
              << std::endl;

    consumer.Run();

    return 0;
}
