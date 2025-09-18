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
    data_storage_ = data_storage::DataStorageFactory::CreateDataStorage<QString>(
                                                settings_.data_storage);
    data_storage_->Open();
    data_storage_->SetErrorHandler([](const QString& error) {
        QMessageBox::warning(nullptr, "DataStorage Error",
                             error);
    });
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
