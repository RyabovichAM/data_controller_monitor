#ifndef CSV_DATA_STORAGE_H
#define CSV_DATA_STORAGE_H

#include <QFile>

#include "data_storage_interface.h"
#include "data_storage_settings.h"

namespace data_storage {

template<typename SaveType, typename LoadType = SaveType>
class CsvDataStorage : public DataStorageInterface<SaveType> {
public:
    CsvDataStorage() = default;

    void SetErrorHandler(ErrorHandler handler) override;
    void DataSave(const SaveType& data) override;
    LoadType DataLoad(const QDateTime& from, const QDateTime& to) override;
    bool Open() override;
    bool IsOpen() override;
    void Close() override;

private:
    ErrorHandler error_handler_;
    QTime last_save_time_;
    QDate last_save_day_;
    DataStorageSettings settings_;
    QFile save_file_;
    QTextStream text_stream_;

    void NewDayCheckAndChange();
};

template<typename SaveType, typename LoadType>
void CsvDataStorage<SaveType, LoadType>::SetErrorHandler(ErrorHandler handler) {
    error_handler_ = handler;
}

template<typename SaveType, typename LoadType>
void CsvDataStorage<SaveType, LoadType>::DataSave(const SaveType& data) {
    QTime current_time = QTime::currentTime();
    if(current_time.msecsTo(last_save_time_) < settings_.survey_period) {
        return;
    }
    last_save_time_ = current_time;

    text_stream_ << current_time.toString();
    text_stream_ << data;

    NewDayCheckAndChange();
}

template<typename SaveType, typename LoadType>
LoadType CsvDataStorage<SaveType, LoadType>::DataLoad(const QDateTime& from,
                                                      const QDateTime& to) {

}

template<typename SaveType, typename LoadType>
bool CsvDataStorage<SaveType, LoadType>::Open() {
    save_file_.setFileName(settings_.place_of_save + QDate::currentDate().toString());
    if(!save_file_.open(QIODevice::WriteOnly)) {
        error_handler_(save_file_.errorString());
        return false;
    }
    text_stream_.setDevice(&save_file_);

    return true;
}

template<typename SaveType, typename LoadType>
bool CsvDataStorage<SaveType, LoadType>::IsOpen() {
    return save_file_.isOpen();
}

template<typename SaveType, typename LoadType>
void CsvDataStorage<SaveType, LoadType>::Close() {
    save_file_.close();
}

template<typename SaveType, typename LoadType>
void CsvDataStorage<SaveType, LoadType>::NewDayCheckAndChange() {
    QDate current_day = QDate::currentDate();

    if(last_save_day_ == current_day)
        return;

    save_file_.close();
    save_file_.setFileName(settings_.place_of_save + current_day.toString());
    save_file_.open(QIODevice::WriteOnly);
    last_save_day_ = current_day;
}

}   //data_storage

#endif // CSV_DATA_STORAGE_H
