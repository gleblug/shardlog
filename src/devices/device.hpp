#pragma once

#include "config/config_manager.hpp"
#include "connection/serial.hpp"

#include <condition_variable>
#include <chrono>
#include <atomic>
#include <optional>
#include <mutex>
#include <thread>

namespace chrono = std::chrono;

struct MeasurementResult {
    enum class Status {
        READY,
        TIMEOUT,
        DISCONNECTED,
    } status;
    std::vector<std::pair<std::string, std::string>> values;
};
using MeasurementStatus = MeasurementResult::Status;

class Device {
public:
    explicit Device(const DeviceInfo& info);
    ~Device();
    void stop();
    
    void requestMeasurement();
    bool isMeasuring() const;
    std::string getName() const;
    std::optional<MeasurementResult> getResult();
    // std::vector<std::string> getHeaders() const;

private:
    void measurementThread();
    void reconnect();
    void publishResult(const MeasurementResult& result);
    std::string header(const std::string& title) const;

    Serial connection_;

    std::thread statusThread_;
    std::thread connectionThread_;
    std::atomic_bool reconnectRequested_;

    std::thread measurementThread_;
    std::atomic_bool stopRequested_;
    mutable std::mutex mu_;
    std::atomic_bool measuring_;
    std::atomic_bool measurementRequested_;
    std::condition_variable_any cv_;
    std::optional<MeasurementResult> result_;

    // configs
    std::string name_;
    std::string port_;
    unsigned int boudRate_;

    chrono::milliseconds readTimeout_;
    chrono::milliseconds readWriteDelay_;
    std::string writeSource_;

    CommandList initCommands_;
    NamedCommandList readCommands_;
    NamedCommandList writeCommands_;
    CommandList endCommands_;
};
