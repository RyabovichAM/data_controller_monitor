#include "monitor_unit.h"

#include "transfer_factory.h"

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

void MonitorUnit::SetErrorHandler(ErrorHandler handler) {
    error_handler_ = handler;
}

void MonitorUnit::SetName(const QString& name) {
    mu_unit_name_ = name;
}

const QString& MonitorUnit::Name() const {
    return mu_unit_name_;
}

void MonitorUnit::SetSettings(const MonitorUnitSettings& settings) {
    settings_ = settings;
}

const MonitorUnitSettings& MonitorUnit::Settings() const {
    return settings_;
}

void MonitorUnit::StartTransmission() {
    transfer_ = transfer::TransferFactory::CreateTransfer(settings_.transfer);

    transfer_->SetJsonReceivedDataHandler([self = this](const QJsonDocument& data){
        if(self->observer_)
            self->observer_->Update(data);
    });

    transfer_->Run([self = this](const QString& err){
        if(self->error_handler_)
            self->error_handler_("Unable to start the transmission: " + err);
    });
}

void MonitorUnit::StopTransmission() {
    if(transfer_){
        transfer_->Stop();
        transfer_.reset();
    }
}

void MonitorUnit::Start() {
    StartTransmission();
}

void MonitorUnit::Stop() {
    StopTransmission();
}

}   //app
