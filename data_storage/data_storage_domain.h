#ifndef DATA_STORAGE_DOMAIN_H
#define DATA_STORAGE_DOMAIN_H

#include <QString>
#include <QTime>

namespace data_storage {

struct DataStorageSettings {
    QString place_of_save;
    qint64 survey_period;
};

DataStorageSettings GetDataStorageSettingsFromHashMap(const QHash<QString, QString>& settings_map);

}   //data_storage

#endif // DATA_STORAGE_SETTINGS_H
