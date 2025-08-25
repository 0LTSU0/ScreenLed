#include <string>
#include <stdexcept>
#include <iostream>
#include <atomic>
#include <QObject>
#include <vector>
#include "Commons.h"
#include "algomean.h"
#include "algomedian.h"

#define UDP_PACKET_SIZE 1024
#define MAX_FPS 60 // especially on Linux the performance is so good that we have to throttle down the rate or raspi cant keep up

// Base class for taking screenshots and analyzing RGB data from them. This class implements
// everything cross-platform and screenCaptureWorkerLinux/screenCaptureWorkerWindows
// implement platform specific functionality (aka virtual stuffs)
class screenCaptureWorkerBase : public QObject{
    Q_OBJECT
public:
    virtual void initScreenShotting() = 0;
    virtual void deinitScreenShotting() = 0;
    virtual void takeScreenShot() = 0;
    virtual bool openUDPPort() = 0;
    virtual bool closeUDPPort() = 0;
    virtual void sendRGBData(const char*) = 0;
    virtual void runAnalFunc() = 0;

    bool loadConfigs();
    ScreenCapConfig& getCurrentConfig();
    void updateCurrentConfig(ScreenCapConfig newConf);
    bool createConfigFile();
    const std::string createRGBDataString();

public slots:
    void run();
    void stop(); // TODO check if there is any point in having stop as slot since it just sets m_isRunning=false

public:
    // common vars. Note to self: these need to be public so that the classes deriving this can use them
    std::string m_configPath;
    ScreenCapConfig m_conf;
    std::atomic_bool m_isRunning{false};
    std::vector<rgbValue> m_rgbData;
    int m_keepClipboardSSCtr = 0;
    std::atomic<double> m_fps = 0.0;

    // different algo classes
    AlgoMean m_meanAlgo = AlgoMean();
    AlgoMedian m_medianAlgo = AlgoMedian();
};
