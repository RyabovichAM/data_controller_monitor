#include "application.h"

namespace app {

Application::Application() {

}

MonitorUnit_Iter Application::CreateUnit(const MonitorUnitSettings& settings) {
    mon_units_.emplace_back(settings);
    return --mon_units_.end();
}

}   //app
