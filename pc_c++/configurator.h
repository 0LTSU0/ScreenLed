#pragma once

#include <string>
#include <tuple>

// "main" config
class gConfig
{
private:
	bool keepDebugSS; //keep screenshot on clipboard for debugging?
	int debugSSInterval; //every Nth screenshot will be stored on clipboard
	std::string raspiIp;
	int raspiPort;

public:
	void readConfig(const char* path);
	const char* getRaspiIp();
	int getRaspiPort();
	bool getKeepDebugSS();
	int getDebugSSInterval();
};

class instanceConfCommon
{
private:

public:
	int res_x = 0;
	int res_y = 0;
	bool centerThird = true;
	void loadInstanceConfCommon(const char* path);
};
