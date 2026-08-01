#include "file_data_storage_domain.h"

namespace data_storage {

DataFormat GetDataFormatFromStr(const QString& frm) {
    if(frm == "text") {
        return DataFormat::TEXT;
    }
    if(frm == "binary") {
        return DataFormat::BINARY;
    }

    qFatal("data_storage::GetDataFormatFromstr: wrong DataFormat from string");
    return {};
}

FileDataStorageSettings GetFileDataStorageSettingsFromHashMap(const QHash<QString, QString>& settings_map) {
    FileDataStorageSettings settings;
    settings.place_of_save = settings_map["location"];
    settings.survey_period = settings_map["period"].toLongLong();
    settings.data_format = GetDataFormatFromStr(settings_map["data_format"]);
    return settings;
}

}   //data_storage
