#include "storage_registry.h"

#include <filesystem>
#include <iostream>
#include <utility>

#include "data_storage_factory.h"

namespace data_storage {

namespace {

std::string Value(const Settings& settings, const std::string& key) {
    auto it = settings.find(key);
    return it == settings.end() ? std::string{} : it->second;
}

}   //namespace

StorageRegistry::StorageRegistry(Settings base_settings)
    : base_settings_{std::move(base_settings)}
    , directory_{CollectorDirectoryFactory::Create(base_settings_)} {
}

DataStorageInterface* StorageRegistry::ForCollector(const std::string& collector_id) {
    std::lock_guard<std::mutex> lock{mutex_};

    auto it = storages_.find(collector_id);
    if (it != storages_.end()) {
        return it->second.get();
    }

    // Only the file backend needs a directory of its own; a hypertable row
    // does not.
    if (Value(base_settings_, "type") == "file") {
        std::filesystem::create_directories(
            Value(base_settings_, "root") + "/" + collector_id + "/");
    }

    return Create(collector_id, true);
}

DataStorageInterface* StorageRegistry::FindCollector(const std::string& collector_id) {
    std::lock_guard<std::mutex> lock{mutex_};

    auto it = storages_.find(collector_id);
    if (it != storages_.end()) {
        return it->second.get();
    }

    // Nothing in the map does not mean nothing in storage: after a restart the
    // per-collector objects are built again only as messages or requests
    // arrive.
    if (!directory_->HasCollector(collector_id)) {
        return nullptr;
    }

    return Create(collector_id, false);
}

std::vector<std::string> StorageRegistry::KnownCollectors() const {
    return directory_->KnownCollectors();
}

DataStorageInterface* StorageRegistry::Create(const std::string& collector_id,
                                              bool open_for_writing) {
    auto storage = DataStorageFactory::CreateDataStorage(SettingsFor(collector_id));
    storage->SetErrorHandler([collector_id](const std::string& error) {
        std::cout << "[storage:" << collector_id << "] " << error << std::endl;
    });

    if (open_for_writing) {
        storage->Open();
    }

    return storages_.emplace(collector_id, std::move(storage)).first->second.get();
}

Settings StorageRegistry::SettingsFor(const std::string& collector_id) const {
    Settings settings = base_settings_;
    settings["collector_id"] = collector_id;

    // The file backend keys a storage by a location on disk rather than the id
    // in the settings map; timescaledb reads "collector_id" directly.
    if (Value(base_settings_, "type") == "file") {
        settings["location"] = Value(base_settings_, "root") + "/" + collector_id + "/";
    }

    return settings;
}

}   //data_storage
