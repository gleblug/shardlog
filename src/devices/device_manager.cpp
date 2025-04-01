#include "device_manager.hpp"
#include "config/config_manager.hpp"

DeviceManager::DeviceManager(DataBus dataBus)
    : dataBus_{dataBus}
{}

DeviceManager::~DeviceManager() {}

void DeviceManager::configure(const std::string& experimentName, const std::string& measurementName) {
    const auto& config = ConfigManager::getInstance();
    duration_ = config.getExperimentDuration(experimentName, measurementName);
    timeout_ = config.getExperimentTimeout(experimentName, measurementName);
    statusTimeout_ = config.getExperimentStatusTimeout(experimentName, measurementName);
    for (const auto& deviceInfo : config.getDevices(experimentName, measurementName)) {
        devices_.push_back(std::make_shared<Device>(deviceInfo));
    }
}
