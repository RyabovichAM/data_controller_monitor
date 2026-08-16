#ifndef REALTIME_HUB_H
#define REALTIME_HUB_H

#include <mutex>
#include <string>
#include <unordered_map>

#include <drogon/WebSocketConnection.h>

namespace realtime {

// The open browser connections and what each of them asked for.
//
// Filled from the WebSocket controller, read from the Kafka thread, so the map
// is guarded. Sending is not: trantor's send() hands the write over to the
// loop the connection belongs to when it is called from another thread.
class RealtimeHub {
public:
    // An empty collector_id means "everything", which is what a connection
    // without a query parameter gets.
    void Add(const drogon::WebSocketConnectionPtr& connection, std::string collector_id);
    void Remove(const drogon::WebSocketConnectionPtr& connection);

    // Sends the message to every connection subscribed to that collector.
    void Broadcast(const std::string& collector_id, const std::string& message);

    size_t ConnectionCount() const;

private:
    mutable std::mutex mutex_;
    std::unordered_map<drogon::WebSocketConnectionPtr, std::string> subscriptions_;
};

// Drogon builds controllers itself, so the hub cannot be handed to one through
// a constructor — it is reached through here instead.
RealtimeHub& Hub();

}   //realtime

#endif // REALTIME_HUB_H
