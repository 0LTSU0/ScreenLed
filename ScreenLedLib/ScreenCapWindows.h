#include "ScreenCapBase.h"

class screenCaptureWorkerWindows : screenCaptureWorkerBase {
public:
    screenCaptureWorkerWindows(std::string configPath) {
        m_configPath = configPath;

        if (!loadConfigs()) {
            throw std::runtime_error("Failed to initialize screenCapture worker");
        }
    }

    void screenshotBitMap();
    bool openUDPPort();
private:
};
