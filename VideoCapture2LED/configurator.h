#pragma once

#include <vector>

class c_strip_conf {
public:
	int gpionum; //gpio pin where strip is connected
	int invert; // ??
	std::vector<int> num_leds; //when multiple strips are in series, the vector should have the number of leds in each separate strip
	int brightness; //should always be set to 255
	bool enabled; //is strip enabled?
};

class c_config {
public:
	c_strip_conf strip0_conf;
	c_strip_conf strip1_conf;
	int camera_res_h;
	int camera_res_v;
	int camera_target_fps;
};

class c_configurator {
public:
	c_config configuration;
	bool readConfigJSON(char*);
};