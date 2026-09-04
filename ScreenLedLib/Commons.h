#ifndef COMMONS_H
#define COMMONS_H

#include <map>
#include <string>
#include <vector>
#include <QString>
#include <chrono>

#define NUM_LED_SEGMENTS 20 // TODO: make this adjustable

enum ScreenLedAlgorithm {
    MEAN_DEFAULT,
    MEDIAN,
    FLASH_BOOST
};

enum receiverType {
    DUMMY,
    RASPI_SSH,
    RASPI_MANUAL,
    ESP32
};

enum activeScreenArea {
    FULL,
    CENTER_THIRD,
    AUTO
};

struct rgbValue {
    int r = 0;
    int g = 0;
    int b = 0;
};

struct rgbAnalysisResult {
    std::vector<rgbValue> rgb_values;
    std::chrono::steady_clock::time_point source_ss_timestamp{};
};

struct clientInfo {
    std::string host;
    int port;
    receiverType type;
    std::string ledStripArg;
};

inline std::map<std::string, ScreenLedAlgorithm> algoNameMap{{"Default: mean", ScreenLedAlgorithm::MEAN_DEFAULT},
                                                             {"Median", ScreenLedAlgorithm::MEDIAN},
                                                             {"FlashBoost", ScreenLedAlgorithm::FLASH_BOOST}};

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

inline std::map<activeScreenArea, std::string> screenAnalysisAreaMap{{activeScreenArea::FULL, "Full"},
                                                                  {activeScreenArea::CENTER_THIRD, "Center Third"},
                                                                  {activeScreenArea::AUTO, "Auto"}};

// config struct for ScreenLedLib (NOTE: screenCaptureWorkerBase::createConfigFile() uses this definition for default values)
struct ScreenCapConfig {
    int c_debugSSInterval = 10;
    bool c_keepDebugSSOnClipboard = false;
    std::vector<clientInfo> c_clientInfos = {{"127.0.0.1", 65432, receiverType::DUMMY, ""}};
    bool c_showDebugPreview = false;
    int c_screenResX = 1920;
    int c_screenResY = 1080;
    ScreenLedAlgorithm c_algo = ScreenLedAlgorithm::MEAN_DEFAULT;
    QString c_autorunScriptPath = "";
    std::string c_preferredLocalNetworkInterface = "";
    activeScreenArea c_analyzerScreenArea = activeScreenArea::FULL;
    int c_analyzerDownscaleFactor = 1;
};

inline bool isWindows() {
#ifdef Q_OS_WIN
    return true;
#else
    return false;
#endif
}

struct RawPixelBuffer {
    int width  = 0;
    int height = 0;
    // tightly packed RGB, row-major: buf[(y*width + x)*3 + {0=R,1=G,2=B}]
    std::vector<unsigned char> rgb;
    // timestamp of the image contained in this buffer
    std::chrono::steady_clock::time_point ss_timestamp{};

    inline void getPixel(int x, int y, int& r, int& g, int& b) const {
        const unsigned char* p = &rgb[(static_cast<size_t>(y) * width + x) * 3];
        r = p[0]; g = p[1]; b = p[2];
    }
};

#endif // COMMONS_H
