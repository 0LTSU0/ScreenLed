#include "configurator.h"
#include "json/json.hpp"
#include <fstream>
#include <string>
#include <iostream>
#include <vector>

using json = nlohmann::json;

bool c_configurator::readConfigJSON(char* cPath) 
{
	std::cout << "c_configurator::readConfigJSON(): Reading config from file " << cPath << std::endl;
	
	std::ifstream ifs(cPath);
	if (!ifs)
	{
		std::cout << "Failed to read file!" << std::endl;
		return false;
	}

	json conf = json::parse(ifs);
	json st0 = conf["strip0"];
	json st1 = conf["strip1"];

	//strip configs
	configuration.strip0_conf.gpionum		= st0["gpio"].get<int>();
	configuration.strip0_conf.invert		= st0["invert"].get<int>();
	configuration.strip0_conf.num_leds		= st0["num_leds"].get<std::vector<int>>();
	configuration.strip0_conf.brightness	= st0["brightness"].get<int>();
	configuration.strip0_conf.enabled		= st0["enabled"].get<bool>();
	configuration.strip1_conf.gpionum		= st1["gpio"].get<int>();
	configuration.strip1_conf.invert		= st1["invert"].get<int>();
	configuration.strip1_conf.num_leds		= st1["num_leds"].get<std::vector<int>>();
	configuration.strip1_conf.brightness	= st1["brightness"].get<int>();
	configuration.strip1_conf.enabled		= st1["enabled"].get<bool>();
	
	//general configs
	configuration.camera_res_h				= conf["capture_res_x"].get<int>();
	configuration.camera_res_v				= conf["capture_res_y"].get<int>();
	configuration.camera_target_fps			= conf["capture_target_fps"].get<int>();

	return true;
}