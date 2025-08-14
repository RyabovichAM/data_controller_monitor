#ifndef MONITOR_UNIT_H
#define MONITOR_UNIT_H

#include <memory>
#include <QWidget>

#include "app_domain.h"
#include "transfer_interface.h"

namespace app {

class MonitorUnit
{
public:
    MonitorUnit();
    MonitorUnit(QWidget* widget_ptr,
                const MonitorUnitSettings& settings);

    MonitorUnit(MonitorUnit&&) = default;
    MonitorUnit& operator=(MonitorUnit&&) = default;

    QWidget* GetWidget() const;

private:
    QWidget* view_;
    std::unique_ptr<transfer::TransferInterface> transfer_;
};

}   //app

#endif // MONITOR_UNIT_H
