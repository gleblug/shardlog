#pragma once

#include <yaml-cpp/yaml.h>
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <iostream>
#include <filesystem>
#include <stdexcept>
#include <spdlog/spdlog.h>
#include <unordered_set>
#include <boost/algorithm/string.hpp>
#include <fstream>

namespace lg = spdlog;
namespace fs = std::filesystem;

enum class ReceiverType {
    DIRECTORY,
    TELEGRAM,
    UNKNOWN
};

enum class ReceiverSend {
    DATA,
    STATUS,
    UNKNOWN
};

inline ReceiverType stringToReceiverType(const std::string& str) {
    if (str == "DIRECTORY") return ReceiverType::DIRECTORY;
    if (str == "TELEGRAM") return ReceiverType::TELEGRAM;
    return ReceiverType::UNKNOWN;
}

inline ReceiverSend stringToReceiverSend(const std::string& str) {
    if (str == "DATA") return ReceiverSend::DATA;
    if (str == "STATUS") return ReceiverSend::STATUS;
    return ReceiverSend::UNKNOWN;
}

template<typename K, typename V>
inline std::vector<K> mapKeys(const std::unordered_map<K, V>& map) {
    std::vector<K> keys;
    for (const auto& [key, _] : map) {
        keys.push_back(key);
    }
    return keys;
}

class ConfigManager {
private:
    inline static std::unique_ptr<ConfigManager> instance = nullptr;

// clang-format off
    std::pair<std::string, std::string> exampleExperiment = {
        "example_experiment.yaml",
        R"(# avoid dots in filename
current_measurements:
  duration: 600
  timeout: 1
  status_timeout: 60
  receivers:
    - type: DIRECTORY
      name: example_measurements
      send: DATA
    - type: TELEGRAM
      name: gleblug
      send: STATUS
  devices:
    - name: example_meter
      scheme: current_scheme
      port: /dev/ttyUSB0
      boud_rate: 9600
)"};
    
    std::pair<std::string, std::string> exampleDevice = {
        "example_meter.yaml", 
        R"(# avoid dots in filename
current_scheme:
  read_timeout: 1
  read_write_delay: 0.5
  write_from: schemes/current_data.csv
  commands:
    init:
      - CONFIGURE_TO_DC_CURRENT
    read:
      voltage:
        - READ_VOLTAGE_COMMANDS
      current:
        - READ_CURRENT_COMMANDS
    write:
      voltage:
        - SET_VOLTAGE_COMMANDS {0}
      current:
        - SET_CURRENT_COMMANDS {1}
    end:
      - END_COMMANDS
)"};
// clang-format on

    using ConfigList = std::unordered_map<std::string, YAML::Node>;
    using ConfigFiles = std::unordered_map<std::string, ConfigList>;

    ConfigFiles experimentMeasurements;
    ConfigFiles deviceSchemes;
    
    ConfigManager() {
        loadAllConfigs();
    }
    
    void loadAllConfigs() {
        loadConfigsFromDirectory("experiments", experimentMeasurements, exampleExperiment);
        loadConfigsFromDirectory("devices", deviceSchemes, exampleDevice);
    }
    
    void loadConfigsFromDirectory(
        const std::string& directory,
        ConfigFiles& configs,
        std::pair<std::string, std::string>& exampleConfig
    ) {
        auto path = fs::current_path() / directory;
        
        if (fs::exists(path) && !fs::is_directory(path)) {
            lg:: error("'{}' is not a directory", path.string());
            throw std::runtime_error("configs path is not a directory");
        }

        if (!fs::exists(path)) {
            fs::create_directory(path);
            std::ofstream(path / exampleConfig.first) << exampleConfig.second;
            lg::info("Created '{}' config", path.string());
        }
        
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.is_regular_file() &&
                std::unordered_set<std::string>({".yaml", ".yml"})
                .contains(entry.path().extension())) 
            {
                std::string configName = entry.path().stem().string();
                try {
                    configs[configName] = YAML::LoadFile(entry.path().string()).as<ConfigList>();
                    lg::info("Loaded config: '{}' from '{}'", configName, entry.path().string());
                } catch (const YAML::Exception& e) {
                    lg::error("Error loading config '{}': {}", entry.path().string(), e.what());
                }
            }
        }
    }
    
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

public:
    static ConfigManager& getInstance() {
        if (!instance) {
            instance.reset(new ConfigManager());
        }
        return *instance.get();
    }
    
    [[nodiscard]] std::vector<std::string> getExperimentNames() const {
        return mapKeys(experimentMeasurements);
    }
    
    [[nodiscard]] std::vector<std::string> getDeviceNames() const {
        return mapKeys(deviceSchemes);
    }

    [[nodiscard]] std::vector<std::string> getMeasurementNames(const std::string& experimentName) const {
        return mapKeys(experimentMeasurements.at(experimentName));
    }

    [[nodiscard]] std::vector<std::string> getSchemeNames(const std::string& deviceName) const {
        return mapKeys(deviceSchemes.at(deviceName));
    }

    [[nodiscard]] YAML::Node getMeasurementConfig(const std::string& experimentName, const std::string& measurementName) const {
        return experimentMeasurements.at(experimentName).at(measurementName);
    }
    
    [[nodiscard]] YAML::Node getSchemeConfig(const std::string& deviceName, const std::string& schemeName) const {
        return deviceSchemes.at(deviceName).at(schemeName);
    }
    
    [[nodiscard]] YAML::Node resolveScheme(const std::string& reference) const {
        std::vector<std::string> parts;
        boost::split(parts, reference, boost::is_any_of("."));
        if (parts.size() != 2) {
            throw std::runtime_error("Invalid scheme reference: " + reference);
        }
        return getSchemeConfig(parts.at(0), parts.at(1));
    }
    
    template<typename T>
    [[nodiscard]] T getValue(const YAML::Node& config, const std::string& path, const T& defaultValue = T()) const {
        std::vector<std::string> parts;
        boost::split(parts, path, boost::is_any_of("."));
        
        try {
            auto node = config;
            for (const auto& part : parts) {
                node = node[part];
            }
            return node.as<T>();
        } catch (const YAML::Exception& e) {
            lg::warn("Use default value for '{}', because of '{}'", path, e.what());
            return defaultValue;
        }
    }
    
    struct ReceiverInfo {
        ReceiverType type;
        std::string name;
        ReceiverSend send;
    };
    
    [[nodiscard]] std::vector<ReceiverInfo> getReceivers(const std::string& experimentName, const std::string& measurementName) const {
        std::vector<ReceiverInfo> result;
        auto measurement = getMeasurementConfig(experimentName, measurementName);
        auto receivers = measurement["receivers"].as<std::vector<YAML::Node>>();
        for (const auto& receiver : receivers) {
            ReceiverInfo info;
            info.type = stringToReceiverType(receiver["type"].as<std::string>());
            info.name = receiver["name"].as<std::string>();
            info.send = stringToReceiverSend(receiver["send"].as<std::string>());
            result.push_back(info);
        }
        return result;
    }
    
    struct DeviceInfo {
        std::string name;
        std::string scheme;
        std::string port;
        unsigned int boudRate;
    };
    
    [[nodiscard]] std::vector<DeviceInfo> getDevices(const std::string& experimentName, const std::string& measurementName) const {
        std::vector<DeviceInfo> result;
        auto measurement = getMeasurementConfig(experimentName, measurementName);
        auto devices = measurement["devices"].as<std::vector<YAML::Node>>();
        for (const auto& device : devices) {
            DeviceInfo info;
            info.name = device["name"].as<std::string>();
            info.scheme = device["scheme"].as<std::string>();
            info.port = device["port"].as<std::string>();
            info.boudRate = device["boud_rate"].as<unsigned int>();
            result.push_back(info);
        }
        return result;
    }
        
    [[nodiscard]] double getExperimentDuration(const std::string& experimentName, const std::string& measurementName) const {
        return getMeasurementConfig(experimentName, measurementName)["duration"].as<double>();
    }
    
    [[nodiscard]] double getExperimentTimeout(const std::string& experimentName, const std::string& measurementName) const {
        return getMeasurementConfig(experimentName, measurementName)["timeout"].as<double>();
    }
    
    [[nodiscard]] double getExperimentStatusTimeout(const std::string& experimentName, const std::string& measurementName) const {
        return getMeasurementConfig(experimentName, measurementName)["status_timeout"].as<double>();
    }
    
    [[nodiscard]] double getSchemeReadTimeout(const std::string& deviceName, const std::string& schemeName) const {
        return getSchemeConfig(deviceName, schemeName)["read_timeout"].as<double>();
    }
    
    [[nodiscard]] double getSchemeReadWriteDelay(const std::string& deviceName, const std::string& schemeName) const {
        return getSchemeConfig(deviceName, schemeName)["read_write_delay"].as<double>();
    }

    [[nodiscard]] std::string getSchemeWriteSource(const std::string& deviceName, const std::string& schemeName) const {
        return getSchemeConfig(deviceName, schemeName)["write_from"].as<std::string>();
    }

    using CommandList = std::vector<std::string>;
    using NamedCommandList = std::vector<std::pair<std::string, CommandList>>;
    
    [[nodiscard]] CommandList getInitCommands(const std::string& deviceName, const std::string& schemeName) const {
        auto config = getSchemeConfig(deviceName, schemeName);
        return config["commands"]["init"].as<CommandList>();
    }
    
    [[nodiscard]] NamedCommandList getReadCommands(const std::string& deviceName, const std::string& schemeName) const {
        auto config = getSchemeConfig(deviceName, schemeName)["commands"]["read"];
        NamedCommandList commands;
        for(YAML::const_iterator it = config.begin(); it != config.end(); ++it) {
            commands.push_back(std::make_pair(
                it->first.as<std::string>(),
                it->second.as<CommandList>()
            ));
        }
        return commands;
    }
    
    [[nodiscard]] NamedCommandList getWriteCommands(const std::string& deviceName, const std::string& schemeName) const {
        auto config = getSchemeConfig(deviceName, schemeName)["commands"]["write"];
        NamedCommandList commands;
        for(YAML::const_iterator it = config.begin(); it != config.end(); ++it) {
            commands.push_back(std::make_pair(
                it->first.as<std::string>(),
                it->second.as<CommandList>()
            ));
        }
        return commands;
    }
    
    [[nodiscard]] CommandList getEndCommands(const std::string& deviceName, const std::string& schemeName) const {
        auto config = getSchemeConfig(deviceName, schemeName);
        return config["commands"]["end"].as<CommandList>();
    }
};

using DeviceInfo = ConfigManager::DeviceInfo;
using CommandList = ConfigManager::CommandList;
using NamedCommandList = ConfigManager::NamedCommandList;
