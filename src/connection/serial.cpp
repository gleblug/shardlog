#include "serial.hpp"

#include <iostream>
#include <cstdint>
#include <algorithm>
#include <boost/bind.hpp>

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

using namespace boost;

std::vector<std::string> Serial::allPorts()
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

Serial::Serial() : io(), port(io), timer(io),
timeout(boost::posix_time::seconds(0)), result(), bytesTransferred() {
}

Serial::Serial(const std::string& devname, unsigned int baud_rate,
    asio::serial_port_base::parity opt_parity,
    asio::serial_port_base::character_size opt_csize,
    asio::serial_port_base::flow_control opt_flow,
    asio::serial_port_base::stop_bits opt_stop)
    : io(), port(io), timer(io), timeout(boost::posix_time::seconds(0))
{
    open(devname, baud_rate, opt_parity, opt_csize, opt_flow, opt_stop);
}

void Serial::open(const std::string& devname, unsigned int baud_rate,
    asio::serial_port_base::parity opt_parity,
    asio::serial_port_base::character_size opt_csize,
    asio::serial_port_base::flow_control opt_flow,
    asio::serial_port_base::stop_bits opt_stop)
{
    if (isOpen()) close();
    port.open(devname);
    port.set_option(asio::serial_port_base::baud_rate(baud_rate));
    port.set_option(opt_parity);
    port.set_option(opt_csize);
    port.set_option(opt_flow);
    port.set_option(opt_stop);
}

bool Serial::isOpen() const
{
    return port.is_open();
}

void Serial::close()
{
    if (isOpen() == false) return;
    port.close();
}

void Serial::setTimeout(const boost::posix_time::time_duration& t)
{
    timeout = t;
}

void Serial::write(const char* data, size_t size)
{
    asio::write(port, asio::buffer(data, size));
}

void Serial::write(const std::vector<char>& data)
{
    asio::write(port, asio::buffer(&data[0], data.size()));
}

void Serial::writeString(const std::string& s)
{
    asio::write(port, asio::buffer(s.c_str(), s.size()));
}

void Serial::read(char* data, size_t size)
{
    if (readData.size() > 0)//If there is some data from a previous read
    {
        std::istream is(&readData);
        size_t toRead = std::min(readData.size(), size);//How many bytes to read?
        is.read(data, toRead);
        data += toRead;
        size -= toRead;
        if (size == 0) return;
    }

    setupParameters = ReadSetupParameters(data, size);
    performReadSetup(setupParameters);

    //For this code to work, there should always be a timeout, so the
    //request for no timeout is translated into a very long timeout
    if (timeout != boost::posix_time::seconds(0)) timer.expires_from_now(timeout);
    else timer.expires_from_now(boost::posix_time::hours(100000));

    timer.async_wait(boost::bind(&Serial::timeoutExpired, this,
        asio::placeholders::error));

    result = resultInProgress;
    bytesTransferred = 0;
    for (;;)
    {
        io.run_one();
        switch (result)
        {
        case resultSuccess:
            timer.cancel();
            return;
        case resultTimeoutExpired:
            port.cancel();
            throw(timeout_exception("Timeout expired"));
        case resultError:
            timer.cancel();
            port.cancel();
            throw(boost::system::system_error(boost::system::error_code(),
                "Error while reading"));
        default:
            break;
        }
    }
}

std::vector<char> Serial::read(size_t size)
{
    std::vector<char> result(size, '\0');//Allocate a vector with the desired size
    read(&result[0], size);//Fill it with values
    return result;
}

std::string Serial::readString(size_t size)
{
    std::string result(size, '\0');//Allocate a string with the desired size
    read(&result[0], size);//Fill it with values
    return result;
}

std::string Serial::readStringUntil(const std::string& delim)
{
    // Note: if readData contains some previously read data, the call to
    // async_read_until (which is done in performReadSetup) correctly handles
    // it. If the data is enough it will also immediately call readCompleted()
    setupParameters = ReadSetupParameters(delim);
    performReadSetup(setupParameters);

    //For this code to work, there should always be a timeout, so the
    //request for no timeout is translated into a very long timeout
    if (timeout != boost::posix_time::seconds(0)) timer.expires_from_now(timeout);
    else timer.expires_from_now(boost::posix_time::hours(100000));

    timer.async_wait(boost::bind(&Serial::timeoutExpired, this,
        asio::placeholders::error));

    result = resultInProgress;
    bytesTransferred = 0;
    for (;;)
    {
        io.run_one();
        switch (result)
        {
        case resultSuccess:
        {
            timer.cancel();
            bytesTransferred -= delim.size();//Don't count delim
            std::istream is(&readData);
            std::string result(bytesTransferred, '\0');//Alloc string
            is.read(&result[0], bytesTransferred);//Fill values
            is.ignore(delim.size());//Remove delimiter from stream
            return result;
        }
        case resultTimeoutExpired:
            port.cancel();
            throw(timeout_exception("Timeout expired"));
        case resultError:
            timer.cancel();
            port.cancel();
            throw(boost::system::system_error(boost::system::error_code(),
                "Error while reading"));
            //if resultInProgress remain in the loop
        default:
            break;
        }
    }
}

Serial::~Serial() {
    close();
}

void Serial::performReadSetup(const ReadSetupParameters& param)
{
    if (param.fixedSize)
    {
        asio::async_read(port, asio::buffer(param.data, param.size), boost::bind(
            &Serial::readCompleted, this, asio::placeholders::error,
            asio::placeholders::bytes_transferred));
    }
    else {
        asio::async_read_until(port, readData, param.delim, boost::bind(
            &Serial::readCompleted, this, asio::placeholders::error,
            asio::placeholders::bytes_transferred));
    }
}

void Serial::timeoutExpired(const boost::system::error_code& error)
{
    if (!error && result == resultInProgress) result = resultTimeoutExpired;
}

void Serial::readCompleted(const boost::system::error_code& error,
    const size_t bytesTransferred)
{
    if (!error)
    {
        result = resultSuccess;
        this->bytesTransferred = bytesTransferred;
        return;
    }

    //In case a asynchronous operation is cancelled due to a timeout,
    //each OS seems to have its way to react.
#ifdef _WIN32
    if (error.value() == 995) return; //Windows spits out error 995
#elif defined(__APPLE__)
    if (error.value() == 45)
    {
        //Bug on OS X, it might be necessary to repeat the setup
        //http://osdir.com/ml/lib.boost.asio.user/2008-08/msg00004.html
        performReadSetup(setupParameters);
        return;
    }
#else //Linux
    if (error.value() == 125) return; //Linux outputs error 125
#endif

    result = resultError;
}
