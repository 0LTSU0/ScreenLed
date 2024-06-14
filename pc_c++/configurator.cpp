#include <fstream>
#include <string>
#include <iostream>
#include "configurator.h"
#include "include/json.hpp"

using json = nlohmann::json;

void gConfig::readConfig(const char* path)
{
	// Read general configs from gConfig.json and check that all expected fields are present
	// TODO: should probably use chema to also verify data formats
	std::ifstream ifs(path);
	json conf = json::parse(ifs);
	if ( !(conf.contains("raspiPort") && conf.contains("raspiIp") && conf.contains("keepDebugSS") && conf.contains("debugSSInterval")) ) {
		std::cout << "Cannot start application: gConfig.json is malformed!" << std::endl;
		exit(1);
	}

	int port = conf["raspiPort"].get<int>();
	std::string ip = conf["raspiIp"].get<std::string>();
	bool debugSS = conf["keepDebugSS"].get<bool>();
	int SSinterval = conf["debugSSInterval"].get<int>();

	this->raspiIp = ip;
	this->raspiPort = port;
	this->keepDebugSS = debugSS;
	this->debugSSInterval = SSinterval;
}

const char* gConfig::getRaspiIp()
{
	return this->raspiIp.c_str();
}

int gConfig::getRaspiPort()
{
	return this->raspiPort;
}

bool gConfig::getKeepDebugSS()
{
	return this->keepDebugSS;
}

int gConfig::getDebugSSInterval()
{
	return this->debugSSInterval;
}


void instanceConfCommon::loadInstanceConfCommon(const char* path)
{
	// Read configs that are common for all led workers
	std::ifstream ifs(path);
	json conf = json::parse(ifs);
	if ( !(conf.contains("resolutionX") && conf.contains("resolutionY") && conf.contains("useCenterThird")) ) {
		std::cout << "Cannot start application: iConfig.json is malformed!" << std::endl;
		exit(1);
	}

	this->res_x = conf["resolutionX"].get<int>();
	this->res_y = conf["resolutionY"].get<int>();
	this->centerThird = conf["useCenterThird"].get<bool>();
}
