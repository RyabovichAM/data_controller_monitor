#include "monitor_unit.h"

#include "transfer_factory.h"

namespace app {

MU_ObserverBase::MU_ObserverBase(QObject* parent)
    : QObject{parent} {

}

void MU_ObserverBase::Update(const QJsonDocument& data) {
}

MonitorUnit::MonitorUnit() {}

MonitorUnit::MonitorUnit(const MonitorUnitSettings& settings)
    : settings_{settings} {

}

void MonitorUnit::SetObserver(MU_ObserverBase* observer) {
    observer_ = observer;
}

void MonitorUnit::StartTransmission() {
    transfer_ = transfer::TransferFactory::CreateTransfer(settings_.transfer);
    transfer_->SetJsonReceivedDataHandler([self = this](const QJsonDocument& data){
        if(!self->observer_)
            self->observer_->Update(data);
    });

}

}   //app
