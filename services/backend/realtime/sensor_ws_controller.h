#ifndef SENSOR_WS_CONTROLLER_H
#define SENSOR_WS_CONTROLLER_H

#include <drogon/WebSocketController.h>

namespace realtime {

// The live feed the browser subscribes to: ws://host/ws/sensor-data, with an
// optional ?collector_id= to hear one collector instead of all of them.
//
// Nothing is expected from the client — messages only travel outwards, out of
// the Kafka thread.
class SensorWsController : public drogon::WebSocketController<SensorWsController> {
public:
    void handleNewConnection(const drogon::HttpRequestPtr& request,
                             const drogon::WebSocketConnectionPtr& connection) override;

    void handleNewMessage(const drogon::WebSocketConnectionPtr& connection,
                          std::string&& message,
                          const drogon::WebSocketMessageType& type) override;

    void handleConnectionClosed(const drogon::WebSocketConnectionPtr& connection) override;

    WS_PATH_LIST_BEGIN
    WS_PATH_ADD("/ws/sensor-data", drogon::Get);
    WS_PATH_LIST_END
};

}   //realtime

#endif // SENSOR_WS_CONTROLLER_H
