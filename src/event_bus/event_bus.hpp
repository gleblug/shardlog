#pragma once

#include <functional>
#include <string>
#include <mutex>

template <typename Event>
class EventBus {
public:
    using Callback = std::function<void(const Event&)>;

    void subscribe(const std::string &id, Callback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        subscribers_[id] = std::move(callback);
    }

    void unsubscribe(const std::string &id) {
        std::lock_guard<std::mutex> lock(mutex_);
        subscribers_.erase(id);
    }

    void publish(const Event &event) const {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& [id, callback]: subscribers_) {
            if (!callback) {
                continue;
            }
            callback(event);
        }
    }

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, Callback> subscribers_;
};
