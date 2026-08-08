#ifndef MONITOR_UNIT_H
#define MONITOR_UNIT_H

#include <functional>
#include <memory>
#include <QJsonDocument>
#include <QString>

#include "app_domain.h"
#include "transfer_interface.h"

namespace app {

class MU_ObserverBase : public QObject {
public:
    MU_ObserverBase(QObject* parent = nullptr);
    virtual void Update(const QJsonDocument& data);
    virtual ~MU_ObserverBase() = default;
};

class MonitorUnit {
public:
    using ErrorHandler = std::function<void(const QString&)>;

    MonitorUnit() = default;
    MonitorUnit(const MonitorUnitSettings& settings);
    ~MonitorUnit();

    MonitorUnit(MonitorUnit&&) = default;
    MonitorUnit& operator=(MonitorUnit&&) = default;

    void SetObserver(MU_ObserverBase* observer);
    void SetErrorHandler(ErrorHandler handler);
    void SetName(const QString& name);
    const QString& Name() const;
    void SetSettings(const MonitorUnitSettings& settings);
    const MonitorUnitSettings& Settings() const;
    void StartTransmission();
    void StopTransmission();

    void Start();
    void Stop();

private:
    MonitorUnitSettings settings_;
    std::unique_ptr<transfer::TransferInterface> transfer_;
    MU_ObserverBase* observer_{nullptr};
    ErrorHandler error_handler_{nullptr};
    QString mu_unit_name_;
};

}   //app

#endif // MONITOR_UNIT_H
