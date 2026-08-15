#include "storage_registry.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <utility>

#include "data_storage_factory.h"

namespace data_storage {

namespace fs = std::filesystem;

StorageRegistry::StorageRegistry(std::string root, std::string data_format, std::string period)
    : root_{std::move(root)}
    , data_format_{std::move(data_format)}
    , period_{std::move(period)} {
}

DataStorageInterface* StorageRegistry::ForCollector(const std::string& collector_id) {
    std::lock_guard<std::mutex> lock{mutex_};

    auto it = storages_.find(collector_id);
    if (it != storages_.end()) {
        return it->second.get();
    }

    fs::create_directories(root_ + "/" + collector_id + "/");
    return Create(collector_id, true);
}

DataStorageInterface* StorageRegistry::FindCollector(const std::string& collector_id) {
    std::lock_guard<std::mutex> lock{mutex_};

    auto it = storages_.find(collector_id);
    if (it != storages_.end()) {
        return it->second.get();
    }

    // Nothing in the map does not mean nothing on disk: after a restart the
    // storages are built again only as messages arrive.
    std::error_code error;
    if (!fs::is_directory(root_ + "/" + collector_id, error)) {
        return nullptr;
    }

    return Create(collector_id, false);
}

std::vector<std::string> StorageRegistry::KnownCollectors() const {
    std::vector<std::string> collector_ids;

    std::error_code error;
    for (const auto& entry : fs::directory_iterator{root_, error}) {
        if (entry.is_directory(error)) {
            collector_ids.push_back(entry.path().filename().string());
        }
    }

    std::sort(collector_ids.begin(), collector_ids.end());
    return collector_ids;
}

DataStorageInterface* StorageRegistry::Create(const std::string& collector_id,
                                              bool open_for_writing) {
    Settings settings;
    settings["type"] = "file";
    settings["location"] = root_ + "/" + collector_id + "/";
    settings["data_format"] = data_format_;
    settings["period"] = period_;

    auto storage = DataStorageFactory::CreateDataStorage(settings);
    storage->SetErrorHandler([collector_id](const std::string& error) {
        std::cout << "[storage:" << collector_id << "] " << error << std::endl;
    });

    if (open_for_writing) {
        storage->Open();
    }

    return storages_.emplace(collector_id, std::move(storage)).first->second.get();
}

}   //data_storage
