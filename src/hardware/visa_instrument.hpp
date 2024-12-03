#pragma once
#include "visa/resources.hpp"
#include "visa/commands.hpp"
#include "meter.hpp"

class VisaInstrument : public Meter {
	std::unique_ptr<Resource> res;
	VisaDefaultCommands commands;
public:
	VisaInstrument(std::unique_ptr<Resource> resource, const std::string& name, const MeterType mtype)
		: Meter(name, mtype)
		, res{ std::move(resource) }
		, commands{ mtype }
	{
		xtd::ustring idn(res->idn);
		idn = idn.to_lower();
		if (idn.contains("keysight")) {
			commands = KeysightCommands{ mtype };
		}
		else if (idn.contains("rigol")) {
			commands = RigolCommands{ mtype };
		}
		else {
			lg::warn("Unknown visa device: {}", idn);
		}

		res->write(commands.configure);
		lg::info("instrument configured successfully: IDN={}, configure={}", idn, commands.configure);
	}

	double value() {
		double val = 0;
		auto resp = res->query(commands.read);
		if (!xtd::ustring::try_parse<double>(resp, val)) {
			lg::warn("{}: failed to parse value: {}", res->idn, resp);
		}
		return val;
	}
};