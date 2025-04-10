#include "device.hpp"

#include <spdlog/spdlog.h>
#include <cmath>
#include <format>

namespace lg = spdlog;
using namespace std::chrono_literals;

Device::Device(const DeviceInfo& info)
    : reconnectRequested_{false}
    , stopRequested_{false}
    , measuring_{false}
    , measurementRequested_{false}
    , name_{info.name} 
    , port_{info.port}
    , boudRate_{info.boudRate}
{
    const auto& config = ConfigManager::getInstance();
    readTimeout_ = config.getSchemeReadTimeoutMs(info.name, info.scheme);
    readWriteDelay_ = config.getSchemeReadWriteDelayMs(info.name, info.scheme);
    writeSource_ = config.getSchemeWriteSource(info.name, info.scheme);
    initCommands_ = config.getInitCommands(info.name, info.scheme);
    readCommands_ = config.getReadCommands(info.name, info.scheme);
    writeCommands_ = config.getWriteCommands(info.name, info.scheme);
    endCommands_ = config.getEndCommands(info.name, info.scheme);

    connection_.setTimeout(boost::posix_time::milliseconds(readTimeout_.count()));
    
    lg::info(
        "Device created. Name: '{}', port: '{}', boud_rate: '{}', read_timeout: '{}', read_write_delay: '{}', write_source: '{}', read_commands[0] name: '{}', read_commands[0] command[0]: '{}'",
        name_, port_, boudRate_, readTimeout_.count(), readWriteDelay_.count(), writeSource_, readCommands_[0].first, readCommands_[0].second[0]
    );
    
    DeviceData result;
    try {
        connection_.open(port_, boudRate_);
        result.status = DeviceStatus::READY;
    } catch (const boost::system::system_error& e) {
        lg::error("Failed to open port '{}': '{}'. Trying to reconnect...", port_, e.what());
        reconnect();
        result.status = DeviceStatus::DISCONNECTED;
    }

    publishResult(result);
    measurementThread_ = std::thread(&Device::measurementThread, this);
}

Device::~Device() {
    stop();
}

void Device::stop() {
    if (stopRequested_) return;
    {
        std::unique_lock lock(mu_);
        stopRequested_ = true;
        reconnectRequested_ = false;
        cv_.notify_all();
    }

    if (measurementThread_.joinable()) {
        measurementThread_.join();
    }

    if (connectionThread_.joinable()) {
        connectionThread_.join();
    }
}

void Device::requestMeasurement() {
    if (measuring_) return;
    measuring_ = true;
    measurementRequested_ = true;
    cv_.notify_all();
}

bool Device::isMeasuring() const {
    return measuring_;
}

std::string Device::getName() const {
    return name_;
}

std::optional<DeviceData> Device::getResult() {
    std::unique_lock lock(mu_);
    if (!result_.has_value()) return std::nullopt;
    measuring_ = false;
    return std::exchange(result_, std::nullopt);
}

// std::vector<std::string> Device::getHeaders() const {
//     std::vector<std::string> headers;
//     for (const auto& [title, _]: readCommands_) {
//         headers.push_back(header(title));
//     }
//     return headers;
// }

void Device::measurementThread() {
    while (true) {
        {
            std::unique_lock lock(mu_);
            if (stopRequested_) break;
            cv_.wait(lock, [this] {
                return measurementRequested_ || stopRequested_;
            });
            if (stopRequested_) break;
            
            measurementRequested_ = false;
        }

        DeviceData result{DeviceStatus::READY, {}};
        for (const auto& [title, commandList]: readCommands_) {
            std::string value;
            try {
                for (const auto& command: commandList) {
                    connection_.writeString(command + "\n");
                }
                value = connection_.readStringUntil();
            }
            catch (const timeout_exception&) {
                result = DeviceData{DeviceStatus::TIMEOUT, {}};
                break;
            }
            catch (const boost::system::system_error&) {
                result = DeviceData{DeviceStatus::DISCONNECTED, {}};
                reconnect();
                break;
            }
            result.values.emplace_back(header(title), value);
        }

        publishResult(result);
    }
}

void Device::reconnect() {
    if (reconnectRequested_) return;
    reconnectRequested_ = true;

    if (connectionThread_.joinable()) {
        connectionThread_.join();
    }
    connectionThread_ = std::thread([this] {
        while (reconnectRequested_) {
            try {
                connection_.open(port_, boudRate_);
            }
            catch (const boost::system::system_error&) {
                lg::error("Failed to reconnect port '{}'", port_);
            }
            if (connection_.isOpen()) {
                lg::info("Port '{}' reconnected", port_);
                reconnectRequested_ = false;
                break;
            }
            std::unique_lock lock(mu_);
            cv_.wait_for(lock, 5s);
        }
    });
}

void Device::publishResult(const DeviceData& result) {
    std::unique_lock lock(mu_);
    result_ = result;
}

std::string Device::header(const std::string& valueTitle) const {
    return std::format("{}.{}", name_, valueTitle);
}
