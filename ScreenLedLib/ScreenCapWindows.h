#include "ScreenCapBase.h"

class screenCaptureWorkerWindows : screenCaptureWorkerBase {
public:
    screenCaptureWorkerWindows(std::string configPath) {
        m_configPath = configPath;

        if (!initScreenCaptureWorker()) {
            throw std::runtime_error("Failed to initialize screenCapture wokrer");
        }
    }

    void screenshotBitMap();
    bool openUDPPort();
private:
};
