#include "monitor_unit.h"

namespace app {

MonitorUnit::MonitorUnit() {}

MonitorUnit::MonitorUnit(QWidget* widget_ptr,
            const MonitorUnitSettings& settings)
    : view_{widget_ptr} {

}

QWidget* MonitorUnit::GetWidget() const {
    return view_;
}

}   //app
