#include "ScreenCapWindows.h"
#include "QDebug"

void screenCaptureWorkerWindows::takeScreenShot() {
    int res_x = m_conf.c_screenResX;
    int res_y_third = m_conf.c_screenResY / 3;

    HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(m_memoryDC, m_bitmap));
    BitBlt(m_memoryDC, 0, 0, res_x, res_y_third, m_screenDC, 0, res_y_third, SRCCOPY);
    m_bitmap = static_cast<HBITMAP>(SelectObject(m_memoryDC, hOldBitmap));

    if (m_conf.c_keepDebugSSOnClipboard) {
        if (m_keepClipboardSSCtr == m_conf.c_debugSSInterval) {
            HDC hScreen = GetDC(NULL);
            HDC hSrcDC  = CreateCompatibleDC(hScreen);
            HDC hTempDC = CreateCompatibleDC(hScreen);
            HBITMAP hOldSrc = static_cast<HBITMAP>(SelectObject(hSrcDC, m_bitmap));

            // Create a proper DDB
            HBITMAP hCopy = CreateCompatibleBitmap(hScreen, res_x, res_y_third);
            HBITMAP hOldTemp = static_cast<HBITMAP>(SelectObject(hTempDC, hCopy));

            // Copy pixels from your working DC
            BitBlt(hTempDC, 0, 0, res_x, res_y_third, hSrcDC, 0, 0, SRCCOPY);

            SelectObject(hSrcDC, hOldSrc);
            SelectObject(hTempDC, hOldTemp);
            DeleteDC(hSrcDC);
            DeleteDC(hTempDC);
            ReleaseDC(NULL, hScreen);

            if (OpenClipboard(NULL)) {
                EmptyClipboard();
                if (!SetClipboardData(CF_BITMAP, hCopy)) {
                    qDebug() << "Failed to set clipboard contents:" << GetLastError();
                    DeleteObject(hCopy); // only delete if SetClipboardData failed
                }
                CloseClipboard();
            }
            m_keepClipboardSSCtr = 0;
        } else {
            m_keepClipboardSSCtr++;
        }
    }

    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = res_x;
    bmi.bmiHeader.biHeight = res_y_third;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32; // Assuming 32-bit color depth
    GetDIBits(m_memoryDC, m_bitmap, 0, res_y_third, m_pixelData.get(), &bmi, DIB_RGB_COLORS);
}

void screenCaptureWorkerWindows::initScreenShotting() {
    int res_x = m_conf.c_screenResX;
    int centerThirdY = m_conf.c_screenResY / 3;
    m_pixelData = std::make_shared<DWORD[]>(m_conf.c_screenResX * centerThirdY);
    m_screenDC  = GetDC(nullptr);
    m_memoryDC  = CreateCompatibleDC(m_screenDC);
    m_bitmap  = CreateCompatibleBitmap(m_screenDC, res_x, centerThirdY);
}

void screenCaptureWorkerWindows::deinitScreenShotting() {
    DeleteObject(m_bitmap);
    DeleteDC(m_memoryDC);
    ReleaseDC(nullptr, m_screenDC);
}

void screenCaptureWorkerWindows::sendRGBData(const char* buf) {
    int res = sendto(m_sock, buf, (int)strlen(buf), 0, (sockaddr*)&m_destAddr, sizeof(m_destAddr));
    if (res == SOCKET_ERROR) {
        std::cerr << "sendto failed: " << WSAGetLastError() << std::endl;
    }
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

void screenCaptureWorkerWindows::runAnalFunc() {
    switch(m_conf.c_algo) {
    case ScreenLedAlgorithm::MEAN_DEFAULT:
        m_meanAlgo.analyzeColors(m_rgbData, m_conf, m_pixelData);
        break;
    case ScreenLedAlgorithm::MEDIAN:
        m_medianAlgo.analyzeColors(m_rgbData, m_conf, m_pixelData);
        break;
    }
}
