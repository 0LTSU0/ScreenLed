#include "ScreenCapBase.h"
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>

#pragma comment(lib, "ws2_32.lib")
#define PACKET_SIZE 1024

class screenCaptureWorkerWindows : public screenCaptureWorkerBase {
public:
    screenCaptureWorkerWindows(std::string configPath) {
        m_configPath = configPath;

        if (!loadConfigs()) {
            throw std::runtime_error("Failed to initialize screenCapture worker");
        }
    }
    ~screenCaptureWorkerWindows() {
        closeUDPPort();
    }

    void screenshotBitMap();
    bool openUDPPort();
    bool closeUDPPort();

private:
    SOCKET m_sock;
    sockaddr_in m_destAddr;
    bool m_sockOpen = false;
    //DWORD* pixelData = new DWORD[]
};
