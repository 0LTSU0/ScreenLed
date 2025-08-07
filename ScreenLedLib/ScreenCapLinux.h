#include "ScreenCapBase.h"

class screenCaptureWorkerLinux : public screenCaptureWorkerBase {
public:
    screenCaptureWorkerLinux(std::string configPath) {
        m_configPath = configPath;

        if (!loadConfigs()) {
            throw std::runtime_error("Failed to initialize screenCapture worker");
        }
    }
    ~screenCaptureWorkerLinux() {
        closeUDPPort();
    }

    void initScreenShotting();
    void deinitScreenShotting();
    void takeScreenShot();
    void analyzeColors();
    void sendRGBData(const char* buf);
    bool openUDPPort();
    bool closeUDPPort();
};
