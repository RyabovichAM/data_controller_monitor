#include "watch_registry.h"

#include <utility>
#include <vector>

namespace config {

WatchSubscriber::Event WatchSubscriber::Wait(std::chrono::milliseconds timeout,
                                             CollectorConfig* config) {
    std::unique_lock<std::mutex> lock{mutex_};
    updated_.wait_for(lock, timeout, [this]() {
        return deleted_ || pending_.has_value();
    });

    // Deletion wins over a pending config: there is nothing to reconfigure to.
    if (deleted_) {
        return Event::kDeleted;
    }

    if (!pending_) {
        return Event::kTimeout;
    }

    CollectorConfig next = std::move(*pending_);
    pending_.reset();

    // Already in the stream: either written from the initial read, or announced
    // by a save that lost the race to a later one.
    if (next.version() <= delivered_version_) {
        return Event::kTimeout;
    }

    if (config) {
        *config = std::move(next);
    }

    return Event::kConfig;
}

void WatchSubscriber::MarkDelivered(int64_t version) {
    std::lock_guard<std::mutex> lock{mutex_};
    if (version > delivered_version_) {
        delivered_version_ = version;
    }
}

void WatchSubscriber::PushConfig(const CollectorConfig& config) {
    {
        std::lock_guard<std::mutex> lock{mutex_};
        pending_ = config;
    }
    updated_.notify_one();
}

void WatchSubscriber::PushDeleted() {
    {
        std::lock_guard<std::mutex> lock{mutex_};
        deleted_ = true;
    }
    updated_.notify_one();
}

WatchRegistry::Subscription::Subscription(WatchRegistry& registry,
                                          const std::string& collector_id)
    : registry_{registry}
    , collector_id_{collector_id}
    , subscriber_{registry.Add(collector_id)} {
}

WatchRegistry::Subscription::~Subscription() {
    registry_.Remove(collector_id_, subscriber_);
}

WatchSubscriber& WatchRegistry::Subscription::Subscriber() {
    return *subscriber_;
}

void WatchRegistry::NotifyChanged(const CollectorConfig& config) {
    std::vector<std::shared_ptr<WatchSubscriber>> targets;
    {
        std::lock_guard<std::mutex> lock{mutex_};
        auto range = subscribers_.equal_range(config.collector_id());
        for (auto it = range.first; it != range.second; ++it) {
            targets.push_back(it->second);
        }
    }

    // Notified outside the registry lock: a subscriber only ever touches its
    // own mutex, and holding two locks at once is not worth the risk.
    for (const auto& subscriber : targets) {
        subscriber->PushConfig(config);
    }
}

void WatchRegistry::NotifyDeleted(const std::string& collector_id) {
    std::vector<std::shared_ptr<WatchSubscriber>> targets;
    {
        std::lock_guard<std::mutex> lock{mutex_};
        auto range = subscribers_.equal_range(collector_id);
        for (auto it = range.first; it != range.second; ++it) {
            targets.push_back(it->second);
        }
    }

    for (const auto& subscriber : targets) {
        subscriber->PushDeleted();
    }
}

std::shared_ptr<WatchSubscriber> WatchRegistry::Add(const std::string& collector_id) {
    auto subscriber = std::make_shared<WatchSubscriber>();

    std::lock_guard<std::mutex> lock{mutex_};
    subscribers_.emplace(collector_id, subscriber);

    return subscriber;
}

void WatchRegistry::Remove(const std::string& collector_id,
                           const std::shared_ptr<WatchSubscriber>& subscriber) {
    std::lock_guard<std::mutex> lock{mutex_};

    auto range = subscribers_.equal_range(collector_id);
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second == subscriber) {
            subscribers_.erase(it);
            return;
        }
    }
}

}   //config
