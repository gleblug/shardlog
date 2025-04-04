#pragma once

#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include <atomic>
#include <thread>
#include <condition_variable>

#include "devices/device.hpp"
#include "events/data_event.hpp"

namespace chrono = std::chrono;

class DeviceManager {
public:
	explicit DeviceManager(DataBus dataBus);
    ~DeviceManager();

	void configure(const std::string& experimentName, const std::string& measurementName);
	void start();
	void stop();
	
private:
	void poolThread();

	DataBus dataBus_;
	std::vector<std::shared_ptr<Device>> devices_;

	std::atomic_bool configured_;
	std::atomic_bool running_;
	std::atomic_bool stopRequested_;
	std::thread poolThread_;
	std::mutex mu_;
	std::condition_variable cv_;

	// configs
	chrono::milliseconds duration_;
	chrono::milliseconds timeout_;
	chrono::milliseconds statusTimeout_;
};
