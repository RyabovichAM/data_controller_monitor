#ifndef DATA_STORAGE_FACTORY_H
#define DATA_STORAGE_FACTORY_H

#include <memory>

#include "data_storage_domain.h"
#include "data_storage_interface.h"

namespace data_storage {

class DataStorageFactory {
public:
    // Throws std::invalid_argument on a type nobody can build: a service
    // without a storage has nothing to do anyway.
    static std::unique_ptr<DataStorageInterface> CreateDataStorage(const Settings& settings);
};

}   //data_storage

#endif // DATA_STORAGE_FACTORY_H
