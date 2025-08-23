#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QWidget>

#include "app_domain.h"
#include "monitor_unit.h"

namespace app {

using MonitorUnit_Iter = std::list<MonitorUnit>::iterator;

class Application : public QObject
{
    Q_OBJECT
public:
    Application();

    MonitorUnit_Iter CreateUnit(const MonitorUnitSettings& settings);

private:
    std::list<MonitorUnit> mon_units_;
};

}   //app

#endif // APPLICATION_H
