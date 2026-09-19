#include "Commons.h"
#include "ScreenLedLib.h"
#include <iostream>
#include <thread>
#include <chrono>

ScreenCapConfig create_test_config()
{
	auto config = ScreenCapConfig();
	config.c_algo = ScreenLedAlgorithm::FLASH_BOOST;
	config.c_screenResX = 3440;
	config.c_screenResY = 1440;
	config.c_analyzerScreenArea = activeScreenArea::CENTER_THIRD;
	config.c_clientInfos.clear();
	config.c_clientInfos.push_back({ "127.0.0.1", 12345, receiverType::DUMMY, "" });
	config.c_keepDebugSSOnClipboard = false;
	return config;
}

int main()
{
	auto conf = create_test_config();

	auto sl_lib = ScreenLedLib(conf);

	std::cout << "Starting ScreenLedLib from main app" << std::endl;
	sl_lib.start();

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::cout << "Screencap FPS: " << sl_lib.getScreenCapFPS() << std::endl;
		std::cout << "Analyzer avg. process time: " << sl_lib.getAnalyzerAnalyzingTime() << std::endl;
		std::cout << "Avg. ss to sent delay" << sl_lib.getAvgSSToSentDelay() << std::endl;
	}

	std::cout << "Stopping ScreenLedLib from main app" << std::endl;
	sl_lib.stop();
	return 0;
}
