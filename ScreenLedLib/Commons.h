#ifndef COMMONS_H
#define COMMONS_H

#include <map>
#include <string>
#include <vector>

#define NUM_LED_SEGMENTS 20 // TODO: make this adjustable

enum ScreenLedAlgorithm {
    MEAN_DEFAULT,
    MEDIAN
};

struct rgbValue {
    int r = 0;
    int g = 0;
    int b = 0;
};

struct clientInfo {
    std::string host;
    int port;
};

inline std::map<std::string, ScreenLedAlgorithm> algoNameMap{{"Default: mean", ScreenLedAlgorithm::MEAN_DEFAULT},
                                                             {"Median", ScreenLedAlgorithm::MEDIAN}};

// config struct for ScreenLedLib (NOTE: screenCaptureWorkerBase::createConfigFile() uses this definition for default values)
struct ScreenCapConfig {
    int c_debugSSInterval = 10;
    bool c_keepDebugSSOnClipboard = false;
    std::vector<clientInfo> c_clientInfos = {{"127.0.0.1", 65432}};
    bool c_showDebugPreview = false;
    int c_screenResX = 1920;
    int c_screenResY = 1080;
    ScreenLedAlgorithm c_algo = ScreenLedAlgorithm::MEAN_DEFAULT;
};



#endif // COMMONS_H
