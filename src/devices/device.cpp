#include "device.hpp"

#include <spdlog/spdlog.h>

namespace lg = spdlog;

Device::Device(const DeviceInfo& info)
    : stopRequested_{false}
    , measuring_{false}
    , measurementRequested_{false}
    , name_{info.name} 
    , port_{info.port}
    , boudRate_{info.boudRate}
{
    const auto& config = ConfigManager::getInstance();
    readTimeout_ = chrono::duration<double>(config.getSchemeReadTimeout(info.name, info.scheme));
    readWriteDelay_ = chrono::duration<double>(config.getSchemeReadWriteDelay(info.name, info.scheme));
    writeSource_ = config.getSchemeWriteSource(info.name, info.scheme);
    initCommands_ = config.getInitCommands(info.name, info.scheme);
    readCommands_ = config.getReadCommands(info.name, info.scheme);
    writeCommands_ = config.getWriteCommands(info.name, info.scheme);
    endCommands_ = config.getEndCommands(info.name, info.scheme);

    lg::info(
        "Device created. Name: '{}', port: '{}', boud_rate: '{}', read_timeout: '{}', read_write_delay: '{}', write_source: '{}', read_commands[0] name: '{}', read_commands[0] command[0]: '{}'",
        name_, port_, boudRate_, readTimeout_.count(), readWriteDelay_.count(), writeSource_, readCommands_[0].first, readCommands_[0].second[0]
    );

    // serial.open(port, 9600);
    // serial.setTimeout(boost::posix_time::seconds(1));

    measurementThread_ = std::thread(&Device::measurementThread, this);
}

Device::~Device() {
    stop();
}

void Device::stop() {
    if (stopRequested_) return;
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

std::optional<MeasurementResult> Device::getResult() {
    std::lock_guard lock(mu_);
    if (!result_.has_value()) return std::nullopt;
    measuring_ = false;
    return std::exchange(result_, std::nullopt);
}

void Device::measurementThread() {
    while (true) {
        {
            std::unique_lock lock(mu_);
            cv_.wait(lock, [this] {
                return measurementRequested_ || stopRequested_;
            });

            if (stopRequested_) break;

            measurementRequested_ = false;
        }

        MeasurementResult result{MeasurementStatus::OK, {}};
        for (const auto& [title, commandList]: readCommands_) {
            std::string value;
            try {
                for (const auto& command: commandList) {
                    connection_->writeString(command + "\n");
                }
                value = connection_->readStringUntil();
            }
            catch (const timeout_exception&) {
                result = MeasurementResult{MeasurementStatus::TIMEOUT, {}};
                break;
            }
            catch (const boost::system::system_error&) {
                result = MeasurementResult{MeasurementStatus::DISCONNECTED, {}};
                break;
            }
            result.values.emplace_back(title, value);
        }

        {
            std::unique_lock lock(mu_);
            result_ = result;
        }
    }
}

// void Device::open() {
//     // open serial port
// }
