#ifndef MONITOR_UNIT_H
#define MONITOR_UNIT_H

#include <memory>
#include <QWidget>

#include "app_domain.h"
#include "transfer_interface.h"

namespace app {

class MU_ObserverBase : public QObject {
public:
    MU_ObserverBase(QObject* parent = nullptr);
    virtual void Update(const QJsonDocument& data);
    virtual ~MU_ObserverBase() = default;
};

class MonitorUnit
{
public:
    MonitorUnit();
    MonitorUnit(const MonitorUnitSettings& settings);

    MonitorUnit(MonitorUnit&&) = default;
    MonitorUnit& operator=(MonitorUnit&&) = default;

    void SetObserver(MU_ObserverBase* observer);
    void StartTransmission();
    void StopTransmission();

private:
    MonitorUnitSettings settings_;
    std::unique_ptr<transfer::TransferInterface> transfer_;
    MU_ObserverBase* observer_{nullptr};
};

}   //app

#endif // MONITOR_UNIT_H
