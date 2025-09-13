#ifndef DATA_STORAGE_FACTORY_H
#define DATA_STORAGE_FACTORY_H

#include "data_storage_interface.h"
#include "csv_data_storage.h"

namespace data_storage {

class DataStorageFactory
{
public:
    template<typename SaveType, typename LoadType = SaveType>
    static std::unique_ptr<DataStorageInterface<SaveType,LoadType>>
                CreateDataStorage(const QHash<QString,QString>& settings) {
        if(settings["type"] == "csv") {
            return std::make_unique<CsvDataStorage<SaveType,LoadType>>(settings);
        }
        Q_ASSERT("DataStorageFactory: Unknown storage to create");
        return nullptr;
    }
};

}   //data_storage

#endif // DATA_STORAGE_FACTORY_H
