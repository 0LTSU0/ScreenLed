#pragma once

#include "ScreenCapBase.h"
#include <arpa/inet.h>
#include <unistd.h>

struct sockclient {
    int sock;
    sockaddr_in addr;
};

class screenCaptureWorkerLinux : public screenCaptureWorkerBase {
public:
    screenCaptureWorkerLinux(std::string configPath) {
        m_configPath = configPath;

        if (!loadConfigs()) {
            throw std::runtime_error("Failed to initialize screenCapture worker");
        }
    }
    ~screenCaptureWorkerLinux() {
        closeUDPPorts();
    }

    void initScreenShotting();
    void deinitScreenShotting();
    void takeScreenShot();
    void convertToCommonSSFormat(void* ximage); // Actially XImage*, see below
    void sendRGBData(const char* buf);
    bool openUDPPort(const char* host, int port);
    bool closeUDPPorts();
    void runAnalFunc();

private:
    // X11 and QT have some annoying macro conflicts and the only way I could get it to work is to
    // include the X11 headers in the source file -> cannot use correct types here
    void* m_display = nullptr;      // actually Display*
    unsigned long m_rootWindow = 0; // actually Window
    void* m_image = nullptr;        // actually XImage*

    int m_primaryDisplayOffsetX = 0;
    int m_primaryDisplayOffsetY = 0;
    //int m_sock = -1;
    //sockaddr_in m_outAddr {};
    std::vector<sockclient> m_clientSocks;
};
