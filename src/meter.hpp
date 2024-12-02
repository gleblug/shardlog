#pragma once
#include <better-enums/enum.h>
#include <xtd/ustring.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <utility>

#include "visa_resources.hpp"

namespace lg = spdlog;

BETTER_ENUM(MeterType, uint8_t,
	UNKNOWN = 1,
	VOLT_DC,
	VOLT_AC,
	CURR_DC,
	CURR_AC
)

class Meter {
	MeterType meterType;
public:
	Meter(const MeterType mtype) : meterType(mtype) {}
	virtual ~Meter() = default;
	MeterType type() { return meterType; }
	virtual double value() = 0;
};

class VisaCommandSet {
	std::string stype;
public:
	VisaCommandSet(const MeterType mtype) {
		stype = xtd::ustring::join(":", xtd::ustring(mtype._to_string()).split({ '_' }));
	}
	virtual std::string configure() {
		return ""
			":CONF:" + stype + ";"
			":SAMP:COUN 1;";
	};
	virtual std::string read() {
		return "READ?";
	}
};

class KeysightCommands : public VisaCommandSet {
public:
	KeysightCommands(const MeterType mtype) : VisaCommandSet(mtype) {}
};

class RigolCommands : public VisaCommandSet {
public:
	RigolCommands(const MeterType mtype) : VisaCommandSet(mtype) {}
};

class VisaInstrument : public Meter {
	std::unique_ptr<Resource> res;
	std::unique_ptr<VisaCommandSet> command;
	std::string meterCmd;
public:
	VisaInstrument(std::unique_ptr<Resource> resource, const MeterType mtype) : Meter(mtype), res(std::move(resource)) {
		xtd::ustring idn(res->idn);
		idn = idn.to_lower();
		if (idn.contains("keysight")) {
			command = std::make_unique<KeysightCommands>(mtype);
		}
		else if (idn.contains("rigol")) {
			command = std::make_unique<RigolCommands>(mtype);
		}
		else {
			lg::error("Unsupported visa device: {}", idn);
		}

		res->write(command->configure());
		lg::info("instrument configured successfully: IDN={}, configure={}", idn, command->configure());
	}

	double value() {
		double val = 0;
		auto resp = res->query(meterCmd);
		if (!xtd::ustring::try_parse<double>(resp, val)) {
			lg::warn("{}: failed to parse value: {}", res->idn, resp);
		}
		return val;
	}
};
