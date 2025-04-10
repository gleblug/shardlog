#include "directory.hpp"

#include <fstream>
#include <chrono>
#include <boost/algorithm/string/join.hpp>

#include <spdlog/spdlog.h>

namespace chrono = std::chrono;
namespace lg = spdlog;

Directory::Directory(const std::string& relativePath, DataType send, const std::string& delim)
    : directory_{fs::current_path() / relativePath}
    , sending_{send}
    , delim_{delim}
    , firstReceipt_{false}
{
    if (!fs::exists(directory_) || !fs::is_directory(directory_)) {
        if (fs::exists(directory_)) {
            fs::remove_all(directory_);
        }
        fs::create_directory(directory_);
    }
}

Directory::~Directory() {
    if (file_.is_open()) {
        file_.close();
    }
}

void Directory::receive(const DataEvent& event) {
    if (!file_.is_open()) {
        lg::error("File is not opened in Directory receiver! Call Directory::renew before Directory::receive.");
        return;
    }

    if (firstReceipt_) {
        std::vector<std::string> headers({"time,s"});
        for (const auto& [deviceName, deviceData] : event.results) {
            for (const auto& [title, _] : deviceData.values) {
                headers.push_back(std::format("{},{}", deviceName, title));
            }
        }
        file_ << boost::algorithm::join(headers, delim_) << "\n";
        firstReceipt_ = false;
    }
    
    std::vector<std::string> values;
    auto durationNs = event.timestamp - event.start;
    auto seconds = static_cast<double>(durationNs.count()) / 1e9;
    values.push_back(std::format("{:.3f}", seconds));
    for (const auto& [_, deviceData] : event.results) {
        for (const auto& [_, value] : deviceData.values) {
            values.push_back(value);
        }
    }

    file_ << boost::algorithm::join(values, delim_) << "\n";
}

void Directory::renew(const std::string& name) {
    auto path = directory_ / std::format("{}.csv", name);
    if (fs::exists(path) && !fs::is_regular_file(path)) {
        fs::remove_all(path);
    }

    firstReceipt_ = true;
    file_ = std::ofstream(path);
}
