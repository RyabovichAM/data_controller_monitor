#include "application.h"

namespace app {

Application::Application() {

}

MonitorUnit_Iter Application::CreateUnit(QWidget* widget_ptr,
                                                     const MonitorUnitSettings& settings) {
    mon_units_.emplace_back(widget_ptr, settings);
    return --mon_units_.end();
}

}   //app
