#pragma once

#include "events/data_event.hpp"
#include "receiver.hpp"

#include <memory>
#include <vector>

class ReceiverManager {
private:
    std::vector<std::shared_ptr<IReceiver>> receivers;
public:
    explicit ReceiverManager();
    ~ReceiverManager();

	void configure(const std::string& experimentName, const std::string& measurementName);
    void reset();
    void handleData(const DataEvent& event);
};
