#include "collector_directory.h"

#include <stdexcept>

#include "file_collector_directory.h"
#include "timescale_collector_directory.h"

namespace data_storage {

namespace {

std::string Value(const Settings& settings, const std::string& key) {
    auto it = settings.find(key);
    return it == settings.end() ? std::string{} : it->second;
}

}   //namespace

std::unique_ptr<CollectorDirectory> CollectorDirectoryFactory::Create(const Settings& settings) {
    const std::string kind = Value(settings, "type");

    if (kind == "file") {
        return std::make_unique<FileCollectorDirectory>(Value(settings, "root"));
    }
    if (kind == "timescaledb") {
        return std::make_unique<TimescaleCollectorDirectory>(Value(settings, "dsn"));
    }

    throw std::invalid_argument("data_storage: unknown storage type '" + kind + "'");
}

}   //data_storage
