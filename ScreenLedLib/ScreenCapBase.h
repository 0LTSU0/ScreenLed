#include <string>
#include <stdexcept>
#include <iostream>
#include <atomic>
#include <QObject>

// config struct for ScreenLedLib (NOTE: screenCaptureWorkerBase::createConfigFile() uses this definition for default values)
struct ScreenCapConfig {
    int c_debugSSInterval = 10;
    bool c_keepDebugSSOnClipboard = false;
    std::string c_raspiIp = "127.0.0.1";
    int c_raspiPort = 65432;
    bool c_showDebugPreview = false;
    int c_screenResX = 1920;
    int c_screenResY = 1080;
};

// Base class for taking screenshots and analyzing RGB data from them. This class implements
// everything cross-platform and screenCaptureWorkerLinux/screenCaptureWorkerWindows
// implement platform specific functionality (aka virtual stuffs)
class screenCaptureWorkerBase : public QObject{
    Q_OBJECT
public:
    virtual void screenshotBitMap() = 0;
    virtual bool openUDPPort() = 0;
    virtual bool closeUDPPort() = 0;
    //virtual void sendRGBData() = 0;

    bool loadConfigs();
    ScreenCapConfig& getCurrentConfig();
    void updateCurrentConfig(ScreenCapConfig newConf);
    bool createConfigFile();
    bool analyzeColors();

public slots:
    void run();
    void stop(); // TODO check if there is any point in having stop as slot since it just sets m_isRunning=false

public:
    // common vars. Note to self: these need to be public so that the classes deriving this can use them
    std::string m_configPath;
    ScreenCapConfig m_conf;
    std::atomic_bool m_isRunning{false};
};
