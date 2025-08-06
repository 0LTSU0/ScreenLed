#include "ScreenCapWindows.h"

void screenCaptureWorkerWindows::screenshotBitMap() {
    //TODO
    return;
}

bool screenCaptureWorkerWindows::openUDPPort() {
    std::cout << "Opening UDP Port (Windows)" << std::endl;

    int result = 0;
    WSADATA wsaData;
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return false;
    }
    m_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed reason: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }

    m_destAddr.sin_family = AF_INET;
    m_destAddr.sin_port = htons(m_conf.c_raspiPort);
    inet_pton(AF_INET, m_conf.c_raspiIp.c_str(), &m_destAddr.sin_addr);

    m_sockOpen = true;
    return true;
}

bool screenCaptureWorkerWindows::closeUDPPort() {
    if (m_sockOpen) {
        std::cout << "Closing UDP Port (Windows)" << std::endl;
        closesocket(m_sock);
        WSACleanup();
    }
    m_sockOpen = false;
    return true;
}
