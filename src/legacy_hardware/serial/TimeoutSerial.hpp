#pragma once

#include <stdexcept>
#include <boost/utility.hpp>
#include <boost/asio.hpp>

class timeout_exception : public std::runtime_error
{
public:
    timeout_exception(const std::string& arg) : runtime_error(arg) {}
};

class TimeoutSerial : private boost::noncopyable
{
public:
    TimeoutSerial();

    TimeoutSerial(const std::string& devname, unsigned int baud_rate,
        boost::asio::serial_port_base::parity opt_parity =
        boost::asio::serial_port_base::parity(
            boost::asio::serial_port_base::parity::none),
        boost::asio::serial_port_base::character_size opt_csize =
        boost::asio::serial_port_base::character_size(8),
        boost::asio::serial_port_base::flow_control opt_flow =
        boost::asio::serial_port_base::flow_control(
            boost::asio::serial_port_base::flow_control::none),
        boost::asio::serial_port_base::stop_bits opt_stop =
        boost::asio::serial_port_base::stop_bits(
            boost::asio::serial_port_base::stop_bits::one));

    void open(const std::string& devname, unsigned int baud_rate,
        boost::asio::serial_port_base::parity opt_parity =
        boost::asio::serial_port_base::parity(
            boost::asio::serial_port_base::parity::none),
        boost::asio::serial_port_base::character_size opt_csize =
        boost::asio::serial_port_base::character_size(8),
        boost::asio::serial_port_base::flow_control opt_flow =
        boost::asio::serial_port_base::flow_control(
            boost::asio::serial_port_base::flow_control::none),
        boost::asio::serial_port_base::stop_bits opt_stop =
        boost::asio::serial_port_base::stop_bits(
            boost::asio::serial_port_base::stop_bits::one));

    bool isOpen() const;

    void close();

    void setTimeout(const boost::posix_time::time_duration& t);

    void write(const char* data, size_t size);

    void write(const std::vector<char>& data);

    void writeString(const std::string& s);

    void read(char* data, size_t size);

    std::vector<char> read(size_t size);

    std::string readString(size_t size);

    std::string readStringUntil(const std::string& delim = "\n");

    ~TimeoutSerial();

private:

    class ReadSetupParameters
    {
    public:
        ReadSetupParameters() : fixedSize(false), delim(""), data(0), size(0) {}

        explicit ReadSetupParameters(const std::string& delim) :
            fixedSize(false), delim(delim), data(0), size(0) {
        }

        ReadSetupParameters(char* data, size_t size) : fixedSize(true),
            delim(""), data(data), size(size) {
        }

        //Using default copy constructor, operator=

        bool fixedSize; ///< True if need to read a fixed number of parameters
        std::string delim; ///< String end delimiter (valid if fixedSize=false)
        char* data; ///< Pointer to data array (valid if fixedSize=true)
        size_t size; ///< Array size (valid if fixedSize=true)
    };

    void performReadSetup(const ReadSetupParameters& param);
    void timeoutExpired(const boost::system::error_code& error);
    void readCompleted(const boost::system::error_code& error,
        const size_t bytesTransferred);

    enum ReadResult
    {
        resultInProgress,
        resultSuccess,
        resultError,
        resultTimeoutExpired
    };

    boost::asio::io_service io; ///< Io service object
    boost::asio::serial_port port; ///< Serial port object
    boost::asio::deadline_timer timer; ///< Timer for timeout
    boost::posix_time::time_duration timeout; ///< Read/write timeout
    boost::asio::streambuf readData; ///< Holds eventual read but not consumed
    enum ReadResult result;  ///< Used by read with timeout
    size_t bytesTransferred; ///< Used by async read callback
    ReadSetupParameters setupParameters; ///< Global because used in the OSX fix
};