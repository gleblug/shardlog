#include "device.hpp"

#include <spdlog/spdlog.h>

namespace lg = spdlog;

Device::Device(const DeviceInfo& info)
    : name_{info.name} 
    , port_{info.port}
    , boudRate_{info.boudRate}
    , connection_(info.port, info.boudRate)
{
    const auto& config = ConfigManager::getInstance();
    readTimeout_ = config.getSchemeReadTimeout(info.name, info.scheme);
    readWriteDelay_ = config.getSchemeReadWriteDelay(info.name, info.scheme);
    writeSource_ = config.getSchemeWriteSource(info.name, info.scheme);
    initCommands_ = config.getInitCommands(info.name, info.scheme);
    readCommands_ = config.getReadCommands(info.name, info.scheme);
    writeCommands_ = config.getWriteCommands(info.name, info.scheme);
    endCommands_ = config.getEndCommands(info.name, info.scheme);

    lg::info(
        "Device created. Name: '{}', port: '{}', boud_rate: '{}', read_timeout: '{}', read_write_delay: '{}', write_source: '{}', read_commands[0] name: '{}', read_commands[0] command[0]: '{}'",
        name_, port_, boudRate_, readTimeout_, readWriteDelay_, writeSource_, readCommands_[0].first, readCommands_[0].second[0]
    );
}

void Device::open() {
    // open serial port
}
