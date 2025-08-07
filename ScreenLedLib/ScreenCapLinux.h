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

private:
    // X11 and QT have some annoying macro conflicts and the only way I could get it to work is to
    // include the X11 headers in the source file -> cannot use correct types here
    void* m_display = nullptr;      // actually Display*
    unsigned long m_rootWindow = 0; // actually Window
    void* m_image = nullptr;        // actually XImage*
};
