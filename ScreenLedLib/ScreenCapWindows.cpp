#include "ScreenCapWindows.h"

void screenCaptureWorkerWindows::takeScreenShot() {
    int res_x = m_conf.c_screenResX;
    int res_y_third = m_conf.c_screenResY / 3;

    HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(m_memoryDC, m_bitmap));
    BitBlt(m_memoryDC, 0, 0, res_x, res_y_third, m_screenDC, 0, res_y_third, SRCCOPY);
    m_bitmap = static_cast<HBITMAP>(SelectObject(m_memoryDC, hOldBitmap));

    if (m_conf.c_keepDebugSSOnClipboard) {
        if (m_keepClipboardSSCtr == m_conf.c_debugSSInterval) {
            OpenClipboard(NULL);
            EmptyClipboard();
            SetClipboardData(CF_BITMAP, m_bitmap);
            CloseClipboard();
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
    m_pixelData = std::make_unique<DWORD[]>(m_conf.c_screenResX * centerThirdY);
    m_screenDC  = GetDC(nullptr);
    m_memoryDC  = CreateCompatibleDC(m_screenDC);
    m_bitmap  = CreateCompatibleBitmap(m_screenDC, res_x, centerThirdY);
}

void screenCaptureWorkerWindows::deinitScreenShotting() {
    DeleteObject(m_bitmap);
    DeleteDC(m_memoryDC);
    ReleaseDC(nullptr, m_screenDC);
}

void screenCaptureWorkerWindows::analyzeColors() {
    m_rgbData.clear();
    m_rgbData.assign(NUM_LED_SEGMENTS, rgbValue{});
    int res_x = m_conf.c_screenResX;
    int res_y = m_conf.c_screenResY / 3;
    int xPerSegment = res_x / NUM_LED_SEGMENTS;

    int currentSegment = 0;
    int itemsPerCell = 0;
    DWORD pixelValue;
    for (int yi = 0; yi < res_y; yi += 10) { //loop every 10th line
        currentSegment = 0;
        for (int x = 0; x < res_x; x += 4) { //loop every 4th row WARNING: IF RESOLUTION IS VERY WEIRD THIS WILL MISBEHAVE
            if (x % xPerSegment == 0 && x != 0) { //in every line the row will correspond to all segments
                currentSegment++;
            }
            if (currentSegment == 0) { //track how many items has been put per cell
                itemsPerCell++;
            }

            // Get the pixel value at (x, yi)
            pixelValue = m_pixelData[yi * res_x + x];

            // Extract the red, green, and blue components
            int blue = static_cast<int>(GetRValue(pixelValue));
            int green = static_cast<int>(GetGValue(pixelValue));
            int red = static_cast<int>(GetBValue(pixelValue));

            m_rgbData[currentSegment].r += red;
            m_rgbData[currentSegment].g += green;
            m_rgbData[currentSegment].b += blue;
        }
    }

    //get average based on how many values were used for calculating total
    for (int i=0; i<NUM_LED_SEGMENTS; i++) {
        m_rgbData[i].r /= itemsPerCell;
        m_rgbData[i].g /= itemsPerCell;
        m_rgbData[i].b /= itemsPerCell;
    }
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
