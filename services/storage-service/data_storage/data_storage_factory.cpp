#include "data_storage_factory.h"

#include <stdexcept>

#include "file_data_storage.h"
#include "timescaledb_data_storage.h"

namespace data_storage {

std::unique_ptr<DataStorageInterface> DataStorageFactory::CreateDataStorage(
    const Settings& settings) {
    auto type = settings.find("type");
    const std::string kind = type == settings.end() ? std::string{} : type->second;

    if (kind == "file") {
        return std::make_unique<FileDataStorage>(settings);
    }
    if (kind == "timescaledb") {
        return std::make_unique<TimescaleDbDataStorage>(settings);
    }

    throw std::invalid_argument("data_storage: unknown storage type '" + kind + "'");
}

}   //data_storage
