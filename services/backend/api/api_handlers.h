#ifndef API_HANDLERS_H
#define API_HANDLERS_H

#include <trantor/net/EventLoopThreadPool.h>

#include "clients/storage_client.h"

namespace api {

// Registers the REST endpoints of the backend.
//
// blocking_pool is where the gRPC calls run. Drogon serves requests on its
// event loops, and a synchronous stub sitting on one of them would stall every
// other request and every WebSocket sharing that loop — so the handlers answer
// asynchronously and the waiting happens on these threads instead.
//
// Both references have to outlive the application; in practice they live in
// main() for the whole run.
void RegisterHandlers(clients::StorageClient& storage,
                      trantor::EventLoopThreadPool& blocking_pool);

}   //api

#endif // API_HANDLERS_H
