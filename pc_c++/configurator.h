#pragma once

#include <string>

// "main" config
class gConfig {
private:
	bool keepDebugSS; //keep screenshot on clipboard for debugging?
	int debugSSInterval; //every Nth screenshot will be stored on clipboard
	std::string raspiIp;
	int raspiPort;

public:
	void readConfig();
	const char* getRaspiIp();
	int getRaspiPort();
	bool getKeepDebugSS();
	int getDebugSSInterval();
};
