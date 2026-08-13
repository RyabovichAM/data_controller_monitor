#include "data_storage_domain.h"

namespace data_storage {

namespace {

std::string Value(const Settings& settings, const std::string& key) {
    auto it = settings.find(key);
    return it == settings.end() ? std::string{} : it->second;
}

}   //namespace

DataStorageSettings GetDataStorageSettings(const Settings& settings) {
    DataStorageSettings result;
    result.place_of_save = Value(settings, "location");

    const std::string period = Value(settings, "period");
    result.survey_period = period.empty() ? 0 : std::stoll(period);

    return result;
}

}   //data_storage
