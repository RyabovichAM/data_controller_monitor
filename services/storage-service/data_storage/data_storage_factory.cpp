#include "data_storage_factory.h"

#include <stdexcept>

#include "file_data_storage.h"

namespace data_storage {

std::unique_ptr<DataStorageInterface> DataStorageFactory::CreateDataStorage(
    const Settings& settings) {
    auto type = settings.find("type");

    if (type != settings.end() && type->second == "file") {
        return std::make_unique<FileDataStorage>(settings);
    }

    throw std::invalid_argument("data_storage: unknown storage type '" +
                                (type == settings.end() ? std::string{} : type->second) + "'");
}

}   //data_storage
