#include <string>
#include <stdexcept>
#include <iostream>

// Base class for taking screenshots and analyzing RGB data from them. This class implements
// everything cross-platform and screenCaptureWorkerLinux/screenCaptureWorkerWindows
// implement platform specific functionality (marked virtual here)
class screenCaptureWorkerBase{
public:
    virtual void screenshotBitMap() = 0;
    virtual bool openUDPPort() = 0;
    //virtual void sendRGBData() = 0;

    bool initScreenCaptureWorker();
    bool loadConfigs();
    bool createConfigFile();
    bool analyzeColors();

    // common vars
    std::string m_configPath;
};
