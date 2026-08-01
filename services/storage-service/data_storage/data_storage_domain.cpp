#include "data_storage_domain.h"

namespace data_storage {

DataStorageSettings GetDataStorageSettingsFromHashMap(const QHash<QString, QString>& settings_map) {
    DataStorageSettings settings;
    settings.place_of_save = settings_map["location"];
    settings.survey_period = settings_map["period"].toLongLong();
    return settings;
}

}   //data_storage
