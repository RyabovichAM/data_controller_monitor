#ifndef APP_DOMAIN_H
#define APP_DOMAIN_H

#include "transfer_domain.h"

namespace app {

// The collector configures exactly one unit, so the settings of a unit are the
// settings of its transport — data_storage is gone from here for good, saving
// belongs to storage-service.
using Settings = transfer::Settings;

}   //app

#endif // APP_DOMAIN_H
