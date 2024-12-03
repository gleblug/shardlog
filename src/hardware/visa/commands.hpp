#pragma once

class VisaDefaultCommands {
protected:
	std::string stype;
public:
	std::string configure;
	std::string read;
	VisaDefaultCommands(const MeterType mtype) {
		stype = xtd::ustring::join(":", xtd::ustring(mtype._to_string()).split({ '_' }));
		configure = ""
			":CONF:" + stype + ";";
		read = "READ?";
	}
};

class KeysightCommands : public VisaDefaultCommands {
public:
	KeysightCommands(const MeterType mtype) : VisaDefaultCommands(mtype) {
		configure = ""
			":CONF:" + stype + ";"
			":SAMP:COUN 1;";
		read = ":MEAS:" + stype + "? 1E-3,3E-9;";
	}
};

class RigolCommands : public VisaDefaultCommands {
public:
	RigolCommands(const MeterType mtype) : VisaDefaultCommands(mtype) {
		read = "MEAS:" + stype + "?";
	}
};