#pragma once
#include "visa/resources.hpp"
#include "visa/commands.hpp"
#include "meter.hpp"

class VisaInstrument : public Meter {

	std::unique_ptr<Resource> res;
	VisaDefaultCommands commands;

public:
	VisaInstrument(std::unique_ptr<Resource> resource, const std::string& name, const MeterType mtype);
	double value() final;
};