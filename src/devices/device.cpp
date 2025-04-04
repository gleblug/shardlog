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
    
    MeasurementResult result;
    try {
        connection_.open(port_, boudRate_);
        result.status = MeasurementStatus::READY;
    } catch (const boost::system::system_error& e) {
        lg::error("Failed to open serial port '{}': '{}'", port_, e.what());
        result.status = MeasurementStatus::DISCONNECTED;
    }

    publishResult(result);
    measurementThread_ = std::thread(&Device::measurementThread, this);
}

Device::~Device() {
    stop();
}

void Device::stop() {
    if (stopRequested_) return;
    std::lock_guard lock(mu_);
    stopRequested_ = true;
    cv_.notify_all();

    if (measurementThread_.joinable()) {
        measurementThread_.join();
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

std::optional<MeasurementResult> Device::getResult() {
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

        MeasurementResult result{MeasurementStatus::READY, {}};
        for (const auto& [title, commandList]: readCommands_) {
            std::string value;
            try {
                for (const auto& command: commandList) {
                    connection_.writeString(command + "\n");
                }
                value = connection_.readStringUntil();
            }
            catch (const timeout_exception&) {
                result = MeasurementResult{MeasurementStatus::TIMEOUT, {}};
                break;
            }
            catch (const boost::system::system_error&) {
                result = MeasurementResult{MeasurementStatus::DISCONNECTED, {}};
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
        while (true) {
            try {
                connection_.open(port_, boudRate_);
                break;
            }
            catch (const boost::system::system_error&) {
                std::this_thread::sleep_for(2s);
            }
        }
        lg::info("Serial port '{}' reconnected", port_);
    });
}

void Device::publishResult(const MeasurementResult& result) {
    std::unique_lock lock(mu_);
    result_ = result;
}

std::string Device::header(const std::string& valueTitle) const {
    return std::format("{}.{}", name_, valueTitle);
}
