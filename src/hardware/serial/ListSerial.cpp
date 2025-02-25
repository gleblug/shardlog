#include <iostream>
#include <string>
#include <cstdint>
#include <cstdint>
#include <algorithm>
#if defined (_WIN32) || defined( _WIN64)
#include <Windows.h>
#endif
#if defined(__linux__)
#include <filesystem>
#endif
#if defined(__APPLE__)
#include <stdio.h>
#include <filesystem>
#include <regex>
#endif
#include "ListSerial.hpp"

/*
A simple class to enumerate COM ports on Windows, Linux and Mac OS X
REFERENCES:
https://stackoverflow.com/questions/2674048/what-is-proper-way-to-detect-all-available-serial-ports-on-windows
https://stackoverflow.com/questions/15342804/c-linux-detect-all-serial-ports
https://stackoverflow.com/questions/52164723/how-to-execute-a-command-and-get-return-code-stdout-and-stderr-of-command-in-c
https://stackoverflow.com/questions/3815211/mac-command-line-list-available-serial-ports
*/

ListSerial::ListSerial() = default;

std::vector<std::string> ListSerial::all_ports()
{
	std::vector<std::string> port_list;
#if defined (_WIN32) || defined( _WIN64)
	const uint32_t CHAR_NUM = 1024;
	const uint32_t MAX_PORTS = 255;
	const std::string COM_STR = "COM";
	char path[CHAR_NUM];
	for (uint32_t k = 0; k < MAX_PORTS; k++)
	{
		std::string port_name = COM_STR + std::to_string(k);
		DWORD test = QueryDosDevice(port_name.c_str(), path, CHAR_NUM);
		if (test == 0) continue;
		port_list.push_back(port_name);
	}
#endif
#if defined (__linux__)
	namespace fs = std::filesystem;
    const std::string DEV_PATH = "/dev/serial/by-id";
    try
    {
        fs::path p(DEV_PATH);
        if (!fs::exists(DEV_PATH)) return port_list;
        for (fs::directory_entry de: fs::directory_iterator(p))
        {
            if (fs::is_symlink(de.symlink_status()))
            {
                fs::path symlink_points_at = fs::read_symlink(de);
                port_list.push_back(std::string("/dev/")+symlink_points_at.filename().c_str());
            }
        }
    }
    catch (const fs::filesystem_error &ex) {}
#endif
#if defined(__APPLE__)
	namespace fs = std::filesystem;
	const std::string DEV_PATH = "/dev";
	const std::regex base_regex(R"(\/dev\/(tty|cu)\..*)");
    try
    {
        fs::path p(DEV_PATH);
        if (!fs::exists(DEV_PATH)) return port_list;
        for (fs::directory_entry de: fs::directory_iterator(p)) {
            fs::path canonical_path = fs::canonical(de);
            std::string name = canonical_path.generic_string();
            std::smatch res;
            std::regex_search(name, res, base_regex);
            if (res.empty()) continue;
            port_list.push_back(canonical_path.generic_string());
        }
    }
    catch (const fs::filesystem_error &ex) {}
#endif
	std::sort(port_list.begin(), port_list.end());
	return port_list;
}
