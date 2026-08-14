#pragma once

#include "ScreenCapBase.h"
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>
#include <memory>

#pragma comment(lib, "ws2_32.lib")

struct sockclient {
    SOCKET sock;
    sockaddr_in addr;
};

class screenCaptureWorkerWindows : public screenCaptureWorkerBase {
public:
    screenCaptureWorkerWindows(std::string configPath) {
        m_configPath = configPath;

        if (!loadConfigs()) {
            throw std::runtime_error("Failed to initialize screenCapture worker");
        }
    }
    ~screenCaptureWorkerWindows() {
        closeUDPPorts();
    }

    void initScreenShotting();
    void deinitScreenShotting();
    void takeScreenShot();
    void convertToCommonSSFormat(const std::shared_ptr<DWORD[]>& pixelData);
    void sendRGBData(const char* buffer);
    bool openUDPPort(const char* host, int port);
    bool closeUDPPorts();
    void runAnalFunc();

private:
    std::vector<sockclient> m_clientSocks;
    bool m_socksOpen = false;
    std::shared_ptr<DWORD[]> m_pixelData;

    HDC m_screenDC = nullptr;
    HDC m_memoryDC = nullptr;
    HBITMAP m_bitmap = nullptr;
};
