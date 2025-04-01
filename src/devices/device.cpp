#include "device.hpp"

Device::Device(const DeviceInfo& info)
    : name_{info.name} 
    , port_{info.port}
    , boudRate_{info.boudRate}
{
    const auto& config = ConfigManager::getInstance();
    readTimeout_ = config.getSchemeReadTimeout(info.name, info.scheme);
    readWriteDelay_ = config.getSchemeReadWriteDelay(info.name, info.scheme);
    writeSource_ = config.getSchemeWriteSource(info.name, info.scheme);
    initCommands_ = config.getInitCommands(info.name, info.scheme);
    readCommands_ = config.getReadCommands(info.name, info.scheme);
    writeCommands_ = config.getWriteCommands(info.name, info.scheme);
    endCommands_ = config.getEndCommands(info.name, info.scheme);
}
