#include "connection_manager.hpp"

#include <chrono>
#include <spdlog/spdlog.h>
#include <unordered_set>

using namespace std::chrono_literals;
namespace lg = spdlog;

ConnectionManager::ConnectionManager(ConnectionBus connectionBus)
    : connectionBus_(connectionBus)
    , running_(false)
{}

ConnectionManager::~ConnectionManager() {
    stop();
}

void ConnectionManager::start() {
    if (running_) {
        lg::warn("ConnectionManager is already running");
        return;
    }
    running_ = true;
    thread_ = std::thread(&ConnectionManager::run, this);
    lg::info("ConnectionManager started");
}

void ConnectionManager::stop() {
    if (!running_) {
        lg::warn("ConnectionManager is not running");
        return;
    }
    running_ = false;
    if (thread_.joinable()) {
        thread_.join();
    }
    lg::info("ConnectionManager stopped");
}

void ConnectionManager::run() {
    while(running_) {
        std::this_thread::sleep_for(1s);

        const auto serialPorts = Serial::allPorts();
        std::unordered_set<std::string> oldConnected(connected_);
        std::unordered_set<std::string> newConnected(serialPorts.begin(), serialPorts.end());
        if (oldConnected == newConnected) {
            continue;
        }

        for (const auto& port : newConnected) {
            if (!connected_.contains(port)) {
                connected_.insert(port);
                connectionBus_->publish(ConnectionEvent{
                    ConnectionEvent::Type::CONNECTED,
                    port,
                });
                lg::info("Connected: {}", port);
            }
        }

        for (const auto& port : oldConnected) {
            if (!newConnected.contains(port)) {
                connected_.erase(port);
                connectionBus_->publish(ConnectionEvent{
                    ConnectionEvent::Type::DISCONNECTED,
                    port,
                });
                lg::info("Disconnected: {}", port);
            }
        }
    }
}
