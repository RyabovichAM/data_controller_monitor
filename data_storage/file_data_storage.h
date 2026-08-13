#ifndef CSV_DATA_STORAGE_H
#define CSV_DATA_STORAGE_H

#include "data_storage_interface.h"
#include "file_data_storage_domain.h"

namespace data_storage {

template<typename SaveType, typename LoadType = SaveType>
class FileDataStorage : public DataStorageInterface<SaveType,LoadType> {
public:
    FileDataStorage() = default;
    FileDataStorage(const QHash<QString,QString>& settings);

    void SetErrorHandler(ErrorHandler handler) override;
    void DataSave(const SaveType& data) override;
    LoadType DataLoad(const QDateTime& from, const QDateTime& to) override;
    bool Open() override;
    bool IsOpen() override;
    void Close() override;

private:
    ErrorHandler error_handler_{nullptr};
    QTime last_save_time_;
    QDate last_save_day_;
    FileDataStorageSettings settings_;
    QFile save_file_;

    void NewDayCheckAndChange();
    void SetFileName(QFile& file, const QDate& date);
};

template<typename SaveType, typename LoadType>
FileDataStorage<SaveType, LoadType>::FileDataStorage(const QHash<QString,QString>& settings)
    : settings_{GetFileDataStorageSettingsFromHashMap(settings)}
    , last_save_time_{QTime::currentTime()}
    , last_save_day_{QDate::currentDate()} {
}

template<typename SaveType, typename LoadType>
void FileDataStorage<SaveType, LoadType>::SetErrorHandler(ErrorHandler handler) {
    error_handler_ = handler;
}

template<typename SaveType, typename LoadType>
void FileDataStorage<SaveType, LoadType>::DataSave(const SaveType& data) {
    QTime current_time = QTime::currentTime();

    if(last_save_time_.msecsTo(current_time) < settings_.survey_period) {
        return;
    }
    last_save_time_ = current_time;

    switch(settings_.data_format) {
    case DataFormat::TEXT: {
            QTextStream text_stream(&save_file_);
            text_stream << current_time.toString() << " ";
            text_stream << data << Qt::endl;
            break;
    }
    case DataFormat::BINARY: {
            QDataStream data_stream(&save_file_);
            data_stream << current_time.toString();
            data_stream << data;
            save_file_.flush();
            break;
    }
        default:
            Q_ASSERT("data_storage::FileDataStorage::DataSave: wrong save format");
    }

    NewDayCheckAndChange();
}

template<typename SaveType, typename LoadType>
LoadType FileDataStorage<SaveType, LoadType>::DataLoad(const QDateTime& from,
                                                      const QDateTime& to) {
    if(from > to)
        return {};

    LoadType result;

    QDate from_date = from.date();
    QTime from_time = from.time();
    QDate to_date = to.date();
    QTime to_time = to.time();

    QFile curr_day_file;
    SetFileName(curr_day_file,from_date);

    if(from_date == to_date) {
        if(curr_day_file.open(QIODevice::ReadOnly)) {
            AppendDataFromFile(curr_day_file,result,settings_.data_format,
                               from_date,from_time,to_time);
            curr_day_file.close();

            return result;
        }
    }

    while(from_date < to_date) {
        SetFileName(curr_day_file,from_date);
        if(curr_day_file.open(QIODevice::ReadOnly)) {
            AppendDataFromFile(curr_day_file, result,settings_.data_format, from_date);
            curr_day_file.close();
        }
        from_date = from_date.addDays(1);
    }

    SetFileName(curr_day_file,from_date);
    if(curr_day_file.open(QIODevice::ReadOnly)) {
        AppendDataFromFile(curr_day_file, result, settings_.data_format,
                           from_date, std::nullopt, to_time);
        curr_day_file.close();
    }

    return result;
}

template<typename SaveType, typename LoadType>
bool FileDataStorage<SaveType, LoadType>::Open() {
    SetFileName(save_file_,QDate::currentDate());
    if(!save_file_.open(QIODevice::ReadWrite | QIODevice::Append)) {
        if(error_handler_) {
            error_handler_(save_file_.errorString());
        }
        return false;
    }
    return true;
}

template<typename SaveType, typename LoadType>
bool FileDataStorage<SaveType, LoadType>::IsOpen() {
    return save_file_.isOpen();
}

template<typename SaveType, typename LoadType>
void FileDataStorage<SaveType, LoadType>::Close() {
    save_file_.close();
}

template<typename SaveType, typename LoadType>
void FileDataStorage<SaveType, LoadType>::NewDayCheckAndChange() {
    QDate current_day = QDate::currentDate();

    if(last_save_day_ == current_day)
        return;

    save_file_.close();
    SetFileName(save_file_,current_day);
    if(!save_file_.open(QIODevice::WriteOnly))
        Q_ASSERT("FileDataStorage::NewDayCheckAndChange: file to save do not opened");
    last_save_day_ = current_day;
}

template<typename SaveType, typename LoadType>
void FileDataStorage<SaveType, LoadType>::SetFileName(QFile& file, const QDate& date) {
    switch(settings_.data_format) {
        case DataFormat::TEXT:
            file.setFileName(settings_.place_of_save + date.toString("dd.MM.yyyy") + ".csv");
            break;
        case DataFormat::BINARY:
            file.setFileName(settings_.place_of_save + date.toString("dd.MM.yyyy") + ".dat");
            break;
        default:
            Q_ASSERT("data_storage::FileDataStorage::SetFileName: wrong file foromat");
    }
}

}   //data_storage

#endif // CSV_DATA_STORAGE_H
