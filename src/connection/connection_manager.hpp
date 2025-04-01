#pragma once

#include "serial.hpp"
#include "event_bus/event_bus.hpp"
#include "events/connection_event.hpp"

#include <atomic>
#include <thread>
#include <string>
#include <unordered_set>

class ConnectionManager {
public:
    explicit ConnectionManager(ConnectionBus connectionBus);
    ~ConnectionManager();

    void start();
    void stop();
    //static std::unique_ptr<Serial> open(std::string port);

private:
    void run();

    ConnectionBus connectionBus_;
    std::atomic<bool> running_;
    std::thread thread_;
    std::unordered_set<std::string> connected_;
};
