#ifndef DATA_STORAGE_SETTINGS_H
#define DATA_STORAGE_SETTINGS_H

#include <QString>
#include <QTime>

namespace data_storage {

struct DataStorageSettings {
    QString place_of_save;
    qint64 survey_period;
};

}   //data_storage

#endif // DATA_STORAGE_SETTINGS_H
