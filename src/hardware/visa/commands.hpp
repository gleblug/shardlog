#pragma once

#include <string>

class VisaDefaultCommands {
	std::string query;
public:
	std::string configure;
	std::string read;
	VisaDefaultCommands(const std::string& stype) : query(stype){
		configure = ":CONF:" + query + ";";
		read = ":READ?";
	}
	std::string stype() const {
		return query;
	}
};

class KeysightCommands : public VisaDefaultCommands {
public:
	KeysightCommands(const std::string& stype) : VisaDefaultCommands(stype) {
		configure = ""
			":CONF:" + stype + ";"
			":SAMP:COUN 1;";
		read = ":MEAS:" + stype + "? 1E-3,3E-9;";
	}
};

class RigolCommands : public VisaDefaultCommands {
public:
	RigolCommands(const std::string& stype) : VisaDefaultCommands(stype) {
		read = "MEAS:" + stype + "?";
	}
};