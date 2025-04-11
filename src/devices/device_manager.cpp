#include "device_manager.hpp"
#include "config/config_manager.hpp"

#include <spdlog/spdlog.h>
#include <chrono>
#include <unordered_map>
#include <map>

namespace chrono = std::chrono;
using namespace std::chrono_literals;
namespace lg = spdlog;

DeviceManager::DeviceManager(DataBus dataBus, ConnectionBus connectionBus, CommandBus commandBus)
    : dataBus_{dataBus}
    , connectionBus_{connectionBus}
    , commandBus_{commandBus}
    , configured_{false}
    , running_{false}
    , stopRequested_{false}
{}

DeviceManager::~DeviceManager() {
    stop();
    if (pollThread_.joinable()) pollThread_.join();
}

void DeviceManager::configure(const std::string& experimentName, const std::string& measurementName) {
    configured_ = true;
    devices_.clear();

    const auto& config = ConfigManager::getInstance();
    duration_ = config.getExperimentDuration(experimentName, measurementName);
    timeout_ = config.getExperimentTimeout(experimentName, measurementName);

    for (const auto& deviceInfo : config.getDevices(experimentName, measurementName)) {
        auto device = std::make_shared<Device>(deviceInfo, connectionBus_);
        lastResult_.insert_or_assign(device->getName(), device->getDummyData());
        devices_.push_back(device);
    }
}

void DeviceManager::start() {
    if (!configured_) {
        lg::warn("Trying to start device manager without configuration");
        return;
    }
    if (running_) {
        lg::warn("Trying to start device manager that is already running");
        return;
    }
    
    running_ = true;
    stopRequested_ = false;
        
    if (pollThread_.joinable()) pollThread_.join();
    pollThread_ = std::thread(&DeviceManager::pollThread, this);
}

void DeviceManager::stop() {        
    if (!running_) return;
    running_ = false;
    stopRequested_ = true;
    
    cv_.notify_all();
}

void DeviceManager::pollThread() {
    auto startTime = chrono::steady_clock::now();
    auto endTime = startTime + duration_;
    uint64_t cycleNumber = 0;
    for (auto& device : devices_) {
        device->init();
    }
    while (!stopRequested_) {
        cycleNumber++;
        auto nextStartAfter = startTime + timeout_ * cycleNumber;
        
        auto measurementStart = chrono::steady_clock::now();
        for (auto& device : devices_) {
            // there is no need to check measuring manually
            device->requestMeasurement();
        }

        size_t counter = 0;
        while ((chrono::steady_clock::now() < nextStartAfter) && !stopRequested_) {
            for (auto& device : devices_) {
                auto result = device->getResult();
                if (result) {
                    lastResult_.at(device->getName()) = result.value();
                    ++counter;
                }
            }
            if (counter == devices_.size()) break; // get data from all devices
            std::this_thread::sleep_for(10ms);
        }

        dataBus_->publish({startTime, measurementStart, endTime, lastResult_});

        if (chrono::steady_clock::now() > endTime) {
            commandBus_->publish({CommandType::STOP_MEASUREMENT, {}});
        }

        {
            std::unique_lock lock(mu_);
            if (stopRequested_) break;
            
            auto remaining_time = nextStartAfter - chrono::steady_clock::now();
            if (remaining_time > 0ms) {
                cv_.wait_for(lock, remaining_time, [this] { 
                    return stopRequested_.load();
                });
            }
        }
    }
    for (auto& device : devices_) {
        device->end();
    }
    running_ = false;
}
