#ifndef DATA_STORAGE_INTERFACE_H
#define DATA_STORAGE_INTERFACE_H

#include <QDateTime>

namespace data_storage {

using ErrorHandler = std::function<void(const QString&)>;

template<typename SaveType, typename LoadType = SaveType>
class DataStorageInterface
{
public:
    DataStorageInterface() = default;
    virtual ~DataStorageInterface() = default;

    virtual void SetErrorHandler(ErrorHandler handler) = 0;
    virtual void DataSave(const SaveType& data) = 0;
    virtual LoadType DataLoad(const QDateTime& from, const QDateTime& to) = 0;
    virtual bool Open() = 0;
    virtual bool IsOpen() = 0;
    virtual void Close() = 0;
};

}   //data_storage

#endif // DATA_STORAGE_INTERFACE_H
