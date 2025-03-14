#include "config_parser.hpp"

#include <fstream>
#include <yaml-cpp/yaml.h>
#include <spdlog/spdlog.h>

namespace lg = spdlog;

std::vector<std::string> ConfigParser::measurementNames() {
    auto path = fs::current_path() / "measurements";
    if (!fs::exists(path)) {
        lg::warn("Measurement configs not exists at {}", path.string());
        fs::create_directory(path);
        std::ofstream(path / exampleMeasurement.first) << exampleMeasurement.second;
        lg::warn("Created example measurement config");
        return {};
    }
    return dictNames(path);
}

std::vector<std::string> ConfigParser::schemeNames() {
    auto path = fs::current_path() / "schemes";
    if (!fs::exists(path)) {
        lg::warn("Scheme configs not exists at {}", path.string());
        fs::create_directory(path);
        std::ofstream(path / exampleScheme.first) << exampleScheme.second;
        return {};
    }
    return dictNames(path);
}

std::vector<std::string> ConfigParser::dictNames(fs::path path)
{
    lg::debug("Loading dict files from directory {}", path.string());
    
    if (!fs::exists(path)) {
        lg::info("{} not exists, creating it...", path.string());
        fs::create_directory(path);
        return {};
    }

    std::vector<std::string> res;
    for (const auto & entry : fs::directory_iterator(path)) {
        auto config = YAML::LoadFile(entry.path());
        lg::debug("Loading dict names from {}", entry.path().string());
        for (auto it = config.begin(); it != config.end(); ++it) {
            if (it->second["hidden"] && it->second["hidden"].as<bool>()) {
                continue;
            }
            auto filename = entry.path().filename();;
            auto name = filename.stem().string() + "." + it->first.as<std::string>();
            res.push_back(name);
            lg::debug("Loaded dict name {}", name);
        }
    }
    return res;
}

std::pair<std::string, std::string> ConfigParser::exampleMeasurement = {
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
    - name: my example meter
      port: /dev/ttyUSB0
      scheme: example_meter.current_scheme
)"};

std::pair<std::string, std::string> ConfigParser::exampleScheme = {
    "example_meter.yaml", 
    R"(# avoid dots in filename
current_scheme:
  read_write_delay: 0.5
  timeout: 1
  commands:
    init:
      - CONFIGURE_TO_DC_CURRENT
    read:
      status:
        - READ_STATUS_COMMANDS
      current:
        - READ_CURRENT_COMMANDS
    write:
      from: schemes/current_data.csv
      current:
        - SET_CURRENT_COMMANDS {0}
    end:
      - END_COMMANDS
)"};
