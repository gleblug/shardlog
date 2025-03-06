#pragma once

#include <functional>
#include <string>

template <typename Event>
class EventBus {
public:
    using Callback = std::function<void(const Event&)>;

    void subscribe(const std::string &id, Callback callback) {}
    void unsubscribe(const std::string &id) {}
    void publish(const Event &event) {}

private:
    std::unordered_map<std::string, Callback> subscribers_;
};
