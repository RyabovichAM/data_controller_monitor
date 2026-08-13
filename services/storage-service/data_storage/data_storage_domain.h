#ifndef DATA_STORAGE_DOMAIN_H
#define DATA_STORAGE_DOMAIN_H

#include <cstdint>
#include <string>
#include <unordered_map>

namespace data_storage {

// How a storage is described from the outside — the same flat map the factory
// takes, filled from the environment by the service.
using Settings = std::unordered_map<std::string, std::string>;

struct DataStorageSettings {
    std::string place_of_save;
    int64_t survey_period{0};   // ms between two saved samples, 0 keeps every one
};

DataStorageSettings GetDataStorageSettings(const Settings& settings);

}   //data_storage

#endif // DATA_STORAGE_DOMAIN_H
