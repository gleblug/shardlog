#include "receiver_manager.hpp"

#include "config/config_manager.hpp"
#include "receivers/directory.hpp"

#include <optional>

#include <spdlog/spdlog.h>

namespace lg = spdlog;

std::shared_ptr<IReceiver> makeReceiver(const ReceiverInfo info) {
    switch (info.type) {
    case ReceiverType::DIRECTORY:
        return std::make_shared<Directory>(info.name);
        break;
    default:
        lg::warn("Unknown receiver type");
        return nullptr;
    }
}

void ReceiverManager::configure(const std::string& experimentName, const std::string& measurementName) {
    receivers_.clear();
    baseName_ = std::format("{}_{}", experimentName, measurementName);

    auto& configManager = ConfigManager::getInstance();
    auto receiversInfo = configManager.getReceivers(experimentName, measurementName);
    for (const auto& info : receiversInfo) {
        receivers_.push_back(makeReceiver(info));
    }
}

void ReceiverManager::reset() {
    auto currentTime = chrono::floor<chrono::seconds>(chrono::system_clock::now());
    auto currentName = std::format("{}_{:%Y-%m-%d_%H-%M-%S}", baseName_, currentTime);
    for (const auto& receiver : receivers_) {
        receiver->renew(currentName);
    }
}

void ReceiverManager::handleData(const DataEvent& event) {
    for (const auto& receiver : receivers_) {
        receiver->receive(event);
    }
}
