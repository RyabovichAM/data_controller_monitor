#ifndef WATCH_REGISTRY_H
#define WATCH_REGISTRY_H

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

#include "storage/config_repository.h"

namespace config {

// One live WatchConfig stream. The thread serving the RPC blocks in Wait(),
// SaveConfig and DeleteCollector wake it from whatever thread they run on.
class WatchSubscriber {
public:
    enum class Event {
        kTimeout,   // nothing happened, the caller re-checks cancellation
        kConfig,    // a new config is ready to be written to the stream
        kDeleted    // the collector is gone, the stream has to end
    };

    // Waits for an update no longer than the timeout. The timeout is what makes
    // a disconnected client detectable: only the caller can see cancellation.
    Event Wait(std::chrono::milliseconds timeout, CollectorConfig* config);

    // Told what has already gone into the stream. The stream is subscribed
    // before the first read from the database, so a save committed in between
    // is announced here as well and would otherwise be written twice.
    void MarkDelivered(int64_t version);

    void PushConfig(const CollectorConfig& config);
    void PushDeleted();

private:
    std::mutex mutex_;
    std::condition_variable updated_;
    // Only the newest config is kept: a stream that missed two saves needs the
    // result of the second one, not both of them.
    std::optional<CollectorConfig> pending_;
    int64_t delivered_version_{0};
    bool deleted_{false};
};

// Subscribers grouped by collector id. One id may have several of them — a
// restarted collector whose previous stream is not reaped yet, for instance.
class WatchRegistry {
public:
    // Unsubscribes on destruction, so an RPC that returns early or throws
    // cannot leave a subscriber behind.
    class Subscription {
    public:
        Subscription(WatchRegistry& registry, const std::string& collector_id);
        ~Subscription();

        Subscription(const Subscription&) = delete;
        Subscription& operator=(const Subscription&) = delete;

        WatchSubscriber& Subscriber();

    private:
        WatchRegistry& registry_;
        std::string collector_id_;
        std::shared_ptr<WatchSubscriber> subscriber_;
    };

    void NotifyChanged(const CollectorConfig& config);
    void NotifyDeleted(const std::string& collector_id);

private:
    std::mutex mutex_;
    std::unordered_multimap<std::string, std::shared_ptr<WatchSubscriber>> subscribers_;

    std::shared_ptr<WatchSubscriber> Add(const std::string& collector_id);
    void Remove(const std::string& collector_id,
                const std::shared_ptr<WatchSubscriber>& subscriber);
};

}   //config

#endif // WATCH_REGISTRY_H
