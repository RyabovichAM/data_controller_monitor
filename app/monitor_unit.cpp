#include "monitor_unit.h"

#include <QMessageBox>

#include "transfer_factory.h"
#include "data_storage_factory.h"

namespace app {

MU_ObserverBase::MU_ObserverBase(QObject* parent)
    : QObject{parent} {
}

void MU_ObserverBase::Update(const QJsonDocument& data) {
}

MonitorUnit::MonitorUnit(const MonitorUnitSettings& settings)
    : settings_{settings} {
}

MonitorUnit::~MonitorUnit() {
    Stop();
}

void MonitorUnit::SetObserver(MU_ObserverBase* observer) {
    observer_ = observer;
}

void MonitorUnit::SetName(const QString& name) {
    mu_unit_name_ = name;
}

data_storage::DataStorageInterface<MonitorUnit::DataStorageSaveType, MonitorUnit::DataStorageLoadType>*
        MonitorUnit::DataStorage() const {
    return data_storage_.get();
}

void MonitorUnit::StartTransmission() {
    transfer_ = transfer::TransferFactory::CreateTransfer(settings_.transfer);

    if(data_storage_) {
        transfer_->SetJsonReceivedDataHandler([self = this](const QJsonDocument& data){
            if(self->observer_)
                self->observer_->Update(data);
            self->data_storage_->DataSave(data.toJson(QJsonDocument::Compact));
        });
    } else {
        transfer_->SetJsonReceivedDataHandler([self = this](const QJsonDocument& data){
            if(self->observer_)
                self->observer_->Update(data);
        });
    }
    transfer_->Run([self = this](const QString& err){
        QMessageBox::warning(nullptr, "Transmission Error",
                            "Unable to start the transmission: "
                                 + err);
    });
}

void MonitorUnit::StopTransmission() {
    if(transfer_){
        transfer_->Stop();
        transfer_.reset();
    }
}

void MonitorUnit::InitDataSaving() {
    if(settings_.data_storage["is_enable"] == "not_enable") {
        return;
    }

    settings_.data_storage["location"] += mu_unit_name_ + "/";
    data_storage_ = data_storage::DataStorageFactory::CreateDataStorage<QString,QList<QPair<QDateTime,QJsonDocument>>>(
                                                settings_.data_storage);
    data_storage_->SetErrorHandler([self = this](const QString& error) {
        QMessageBox::warning(nullptr, "DataStorage Error",
                             error);
    });
    data_storage_->Open();
}

void MonitorUnit::DeinitDataSaving() {
    if(data_storage_) {
        data_storage_->Close();
        data_storage_.reset();
    }
}

void MonitorUnit::Start() {
    InitDataSaving();
    StartTransmission();
}

void MonitorUnit::Stop() {
    StopTransmission();
    DeinitDataSaving();
}

}   //app
