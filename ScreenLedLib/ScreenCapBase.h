#include <string>
#include <stdexcept>
#include <iostream>

// config struct for ScreenLedLib (NOTE: screenCaptureWorkerBase::createConfigFile() uses these default values)
struct config {
    int c_debugSSInterval = 10;
    bool c_keepDebugSSOnClipboard = false;
    std::string c_raspiIp = "127.0.0.1";
    int c_raspiPort = 65432;
    bool c_showDebugPreview = false;
};

// Base class for taking screenshots and analyzing RGB data from them. This class implements
// everything cross-platform and screenCaptureWorkerLinux/screenCaptureWorkerWindows
// implement platform specific functionality (marked virtual here)
class screenCaptureWorkerBase{
public:
    virtual void screenshotBitMap() = 0;
    virtual bool openUDPPort() = 0;
    //virtual void sendRGBData() = 0;

    bool loadConfigs();
    bool createConfigFile();
    bool analyzeColors();

    // common vars
    std::string m_configPath;
    config m_conf;
};
