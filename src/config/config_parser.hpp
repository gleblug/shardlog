#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

namespace fs = std::filesystem;




class ConfigParser {
public:
    static std::vector<std::string> measurementNames();
    static std::vector<std::string> schemeNames();

private:
    static std::vector<std::string> dictNames(fs::path path);
    
    static std::pair<std::string, std::string> exampleScheme;
    static std::pair<std::string, std::string> exampleMeasurement;
};
