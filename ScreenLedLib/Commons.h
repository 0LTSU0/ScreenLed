#ifndef COMMONS_H
#define COMMONS_H

#include <map>
#include <string>
#include <vector>
#include <QString>

#define NUM_LED_SEGMENTS 20 // TODO: make this adjustable

enum ScreenLedAlgorithm {
    MEAN_DEFAULT,
    MEDIAN
};

enum receiverType {
    DUMMY,
    RASPI_SSH,
    RASPI_MANUAL,
    ESP32
};

struct rgbValue {
    int r = 0;
    int g = 0;
    int b = 0;
};

struct clientInfo {
    std::string host;
    int port;
    receiverType type;
};

inline std::map<std::string, ScreenLedAlgorithm> algoNameMap{{"Default: mean", ScreenLedAlgorithm::MEAN_DEFAULT},
                                                             {"Median", ScreenLedAlgorithm::MEDIAN}};

inline std::map<std::string, receiverType> receiverTypeNameMap{{"Dummy", receiverType::DUMMY},
                                                               {"Raspi (ssh)", receiverType::RASPI_SSH},
                                                               {"Raspi (manual)", receiverType::RASPI_MANUAL},
                                                               {"ESP32", receiverType::ESP32}};
inline std::map<receiverType, std::string> receiverTypeValueMap{
    {receiverType::DUMMY,        "Dummy"},
    {receiverType::RASPI_SSH,    "Raspi (ssh)"},
    {receiverType::RASPI_MANUAL, "Raspi (manual)"},
    {receiverType::ESP32,        "ESP32"}
};

// config struct for ScreenLedLib (NOTE: screenCaptureWorkerBase::createConfigFile() uses this definition for default values)
struct ScreenCapConfig {
    int c_debugSSInterval = 10;
    bool c_keepDebugSSOnClipboard = false;
    std::vector<clientInfo> c_clientInfos = {{"127.0.0.1", 65432, receiverType::DUMMY}};
    bool c_showDebugPreview = false;
    int c_screenResX = 1920;
    int c_screenResY = 1080;
    ScreenLedAlgorithm c_algo = ScreenLedAlgorithm::MEAN_DEFAULT;
    QString c_autorunScriptPath = "";
};

inline bool isWindows() {
#ifdef Q_OS_WIN
    return true;
#else
    return false;
#endif
}

#endif // COMMONS_H
