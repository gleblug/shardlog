#pragma once

#include "receiver.hpp"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class Directory : public IReceiver {
public:
    Directory(const std::string& relativePath, const std::string& delim = "\t");
    ~Directory();

    void receive(const DataEvent& event) override;
    void renew(const std::string& name) override;

private:
    fs::path directory_;
    std::ofstream file_;

    std::string delim_;
    bool firstReceipt_;
};
