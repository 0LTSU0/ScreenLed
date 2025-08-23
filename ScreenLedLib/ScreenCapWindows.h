#include "ScreenCapBase.h"
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>
#include <memory>

#pragma comment(lib, "ws2_32.lib")

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

    void initScreenShotting();
    void deinitScreenShotting();
    void takeScreenShot();
    void sendRGBData(const char* buffer);
    bool openUDPPort();
    bool closeUDPPort();
    void runAnalFunc();

private:
    SOCKET m_sock;
    sockaddr_in m_destAddr;
    bool m_sockOpen = false;
    std::shared_ptr<DWORD[]> m_pixelData;

    HDC m_screenDC = nullptr;
    HDC m_memoryDC = nullptr;
    HBITMAP m_bitmap = nullptr;
};
