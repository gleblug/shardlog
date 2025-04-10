#pragma once

#include "events/data_event.hpp"
#include "receiver.hpp"

#include <memory>
#include <vector>


class ReceiverManager {
public:
    explicit ReceiverManager() = default;
    ~ReceiverManager() = default;

	void configure(const std::string& experimentName, const std::string& measurementName);
    void reset();
    void handleData(const DataEvent& event);

private:
    std::vector<std::shared_ptr<IReceiver>> receivers_;
    std::string baseName_;
};
