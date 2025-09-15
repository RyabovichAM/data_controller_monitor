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
    : settings_{settings}
    , data_storage_{
        data_storage::DataStorageFactory::CreateDataStorage<QString>(settings.data_storage)} {
    data_storage_->Open();
}

MonitorUnit::~MonitorUnit() {
    StopTransmission();
}

void MonitorUnit::SetObserver(MU_ObserverBase* observer) {
    observer_ = observer;
}

void MonitorUnit::StartTransmission() {
    transfer_ = transfer::TransferFactory::CreateTransfer(settings_.transfer);
    data_storage_->SetErrorHandler([](const QString& error) {
        QMessageBox::warning(nullptr, "DataStorage Error",
                             error);
    });
    transfer_->SetJsonReceivedDataHandler([self = this](const QJsonDocument& data){
        if(self->observer_)
            self->observer_->Update(data);
        self->data_storage_->DataSave(data.toJson(QJsonDocument::Compact));
    });
    transfer_->Run([self = this](const QString& err){
        QMessageBox::warning(nullptr, "Transmission Error",
                            "Unable to start the transmission: "
                                 + err);
    });
}

void MonitorUnit::StopTransmission() {
    transfer_->Stop();
    data_storage_->Close();
}

}   //app
