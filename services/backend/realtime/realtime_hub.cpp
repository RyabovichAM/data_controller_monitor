#include "realtime_hub.h"

#include <utility>
#include <vector>

namespace realtime {

void RealtimeHub::Add(const drogon::WebSocketConnectionPtr& connection,
                      std::string collector_id) {
    std::lock_guard<std::mutex> lock{mutex_};
    subscriptions_[connection] = std::move(collector_id);
}

void RealtimeHub::Remove(const drogon::WebSocketConnectionPtr& connection) {
    std::lock_guard<std::mutex> lock{mutex_};
    subscriptions_.erase(connection);
}

void RealtimeHub::Broadcast(const std::string& collector_id, const std::string& message) {
    std::vector<drogon::WebSocketConnectionPtr> targets;
    {
        std::lock_guard<std::mutex> lock{mutex_};
        for (const auto& [connection, subscription] : subscriptions_) {
            if (subscription.empty() || subscription == collector_id) {
                targets.push_back(connection);
            }
        }
    }

    // Sent outside the lock: the writes go to several event loops, and holding
    // the map while they happen would stall every new connection.
    for (const auto& connection : targets) {
        if (connection->connected()) {
            connection->send(message);
        }
    }
}

size_t RealtimeHub::ConnectionCount() const {
    std::lock_guard<std::mutex> lock{mutex_};
    return subscriptions_.size();
}

RealtimeHub& Hub() {
    static RealtimeHub hub;
    return hub;
}

}   //realtime
