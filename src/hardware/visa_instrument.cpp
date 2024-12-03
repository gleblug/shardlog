#include "visa_instrument.hpp"

#include <memory>
#include <string>

#include <xtd/ustring.h>
#include <spdlog/spdlog.h>

namespace lg = spdlog;

VisaInstrument::VisaInstrument(std::unique_ptr<Resource> resource, const std::string& name, const MeterType mtype)
	: Meter(name, mtype)
	, res{ std::move(resource) }
	, commands{ xtd::ustring::join(":", xtd::ustring(mtype._to_string()).split({ '_' })) }
{
	xtd::ustring idn(res->idn);
	idn = idn.to_lower();
	if (idn.contains("keysight"))
		commands = KeysightCommands{ commands.stype() };
	else if (idn.contains("rigol"))
		commands = RigolCommands{ commands.stype() };
	else
		lg::warn("Unknown visa device!");

	res->write(commands.configure);
	lg::info("Instrument '{}' configured successfully", name);
}

double VisaInstrument::value() {
	double val = 0;
	auto resp = res->query(commands.read);
	if (!xtd::ustring::try_parse<double>(resp, val))
		lg::warn("{}: failed to parse value: {}", name(), resp);

	return val;
}