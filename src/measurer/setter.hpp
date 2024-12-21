#pragma once

#include <thread>
#include <chrono>
#include <vector>
#include <memory>

#include "../meter/meter.hpp"

namespace chrono = std::chrono;

class Setter {
	std::vector<Meter::Ptr> m_meters;
	chrono::duration<double> m_timeout;
	chrono::steady_clock::time_point m_start;
	std::thread m_thread;
	std::atomic_flag m_stop;

public:
	Setter(const std::vector<Meter::Ptr>& meters, chrono::duration<double> timeout = chrono::duration<double>(10e-3))
		: m_meters(meters)
		, m_timeout{timeout}
		, m_thread{}
		, m_stop{} 
	{}

	void start() {
		m_start = chrono::steady_clock::now();
		m_thread = std::thread([this] {
			while (!enough()) {
				setMetersValue();
				std::this_thread::sleep_for(m_timeout);
			}
		});
	}

	void stop() {
		m_stop.test_and_set();
		m_thread.join();
	}

private:
	bool enough() {
		if (m_stop.test())
			return true;
	}

	void setMetersValue() {
		std::vector<std::thread> threads;
		for (const auto& meter : m_meters) {
			if (meter->needToSet(m_start))
				threads.emplace_back([&meter]() { meter->setCurrentData(); });
		}
		for (auto& th : threads)
			th.join();
	}
};