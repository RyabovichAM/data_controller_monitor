#include "sensor_ws_controller.h"

#include <trantor/utils/Logger.h>

#include "realtime_hub.h"

namespace realtime {

void SensorWsController::handleNewConnection(const drogon::HttpRequestPtr& request,
                                             const drogon::WebSocketConnectionPtr& connection) {
    const std::string collector_id = request->getParameter("collector_id");

    Hub().Add(connection, collector_id);

    LOG_INFO << "[ws] subscribed to '" << (collector_id.empty() ? "*" : collector_id)
             << "', connections: " << Hub().ConnectionCount();
}

void SensorWsController::handleNewMessage(const drogon::WebSocketConnectionPtr& /*connection*/,
                                          std::string&& /*message*/,
                                          const drogon::WebSocketMessageType& /*type*/) {
    // The feed is one-way. What the browser wants to change it says over REST,
    // where an answer and a status code are available.
}

void SensorWsController::handleConnectionClosed(
    const drogon::WebSocketConnectionPtr& connection) {
    Hub().Remove(connection);

    LOG_INFO << "[ws] closed, connections: " << Hub().ConnectionCount();
}

}   //realtime
