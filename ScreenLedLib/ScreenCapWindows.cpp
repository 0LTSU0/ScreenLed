#include "ScreenCapWindows.h"
#include "QDebug"
#include <chrono>
#include <immintrin.h>

void screenCaptureWorkerWindows::takeScreenShot() {
    int res_x = m_conf.c_screenResX;
    int res_y = m_conf.c_screenResY;

    HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(m_memoryDC, m_bitmap));
    BitBlt(m_memoryDC, 0, 0, res_x, res_y, m_screenDC, 0, 0, SRCCOPY);
    m_bitmap = static_cast<HBITMAP>(SelectObject(m_memoryDC, hOldBitmap));

    if (m_conf.c_keepDebugSSOnClipboard) {
        if (m_keepClipboardSSCtr == m_conf.c_debugSSInterval) {
            HDC hScreen = GetDC(NULL);
            HDC hSrcDC  = CreateCompatibleDC(hScreen);
            HDC hTempDC = CreateCompatibleDC(hScreen);
            HBITMAP hOldSrc = static_cast<HBITMAP>(SelectObject(hSrcDC, m_bitmap));

            // Create a proper DDB
            HBITMAP hCopy = CreateCompatibleBitmap(hScreen, res_x, res_y);
            HBITMAP hOldTemp = static_cast<HBITMAP>(SelectObject(hTempDC, hCopy));

            // Copy pixels from your working DC
            BitBlt(hTempDC, 0, 0, res_x, res_y, hSrcDC, 0, 0, SRCCOPY);

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
    bmi.bmiHeader.biHeight = res_y;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32; // Assuming 32-bit color depth
    GetDIBits(m_memoryDC, m_bitmap, 0, res_y, m_pixelData.get(), &bmi, DIB_RGB_COLORS);
    auto start = std::chrono::high_resolution_clock::now();
    convertToCommonSSFormat(m_pixelData);
    auto end = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(end - start).count();
    qDebug() << "convertToCommonSSFormat took" << ms << "ms";
}

void convertRowBGRAtoBGR_SSE(const unsigned char* src, unsigned char* dst, int width) {
    const __m128i mask = _mm_setr_epi8(
        0,1,2, 4,5,6, 8,9,10, 12,13,14, -1,-1,-1,-1);

    int x = 0;
    for (; x + 4 <= width; x += 4) {
        __m128i pix = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src + x*4));
        __m128i shuffled = _mm_shuffle_epi8(pix, mask);
        std::memcpy(dst + x*3, &shuffled, 12);
    }
    for (; x < width; ++x) {
        dst[x*3+0] = src[x*4+0];
        dst[x*3+1] = src[x*4+1];
        dst[x*3+2] = src[x*4+2];
    }
}

void screenCaptureWorkerWindows::convertToCommonSSFormat(const std::shared_ptr<DWORD[]>& pixelData)
{
    const int w = m_CommonPixelData.width;
    const int h = m_CommonPixelData.height;
    const int srcStride = m_conf.c_screenResX;
    const unsigned char* src = reinterpret_cast<const unsigned char*>(pixelData.get());
    unsigned char* dst = m_CommonPixelData.rgb.data();

    for (int y = 0; y < h; ++y) {
        convertRowBGRAtoBGR_SSE(src + static_cast<size_t>(y) * srcStride * 4,
                                dst + static_cast<size_t>(y) * w * 3, w);
    }
}

void screenCaptureWorkerWindows::initScreenShotting() {
    int res_x = m_conf.c_screenResX;
    int res_y = m_conf.c_screenResY;
    m_pixelData = std::make_shared<DWORD[]>(m_conf.c_screenResX * res_y);
    m_screenDC  = GetDC(nullptr);
    m_memoryDC  = CreateCompatibleDC(m_screenDC);
    m_bitmap  = CreateCompatibleBitmap(m_screenDC, res_x, res_y);

    m_CommonPixelData.width = res_x;
    m_CommonPixelData.height = res_y;
    m_CommonPixelData.rgb.resize(static_cast<size_t>(m_CommonPixelData.width) * m_CommonPixelData.height * 3);
}

void screenCaptureWorkerWindows::deinitScreenShotting() {
    DeleteObject(m_bitmap);
    DeleteDC(m_memoryDC);
    ReleaseDC(nullptr, m_screenDC);
}

void screenCaptureWorkerWindows::sendRGBData(const char* buf) {
    for (const auto& c : m_clientSocks) {
        int res = sendto(c.sock, buf, (int)strlen(buf), 0, (sockaddr*)&c.addr, sizeof(c.addr));
        if (res == SOCKET_ERROR) {
            std::cerr << "sendto failed: " << WSAGetLastError() << std::endl;
        }
    }
}

bool screenCaptureWorkerWindows::openUDPPort(const char* host, int port) {
    std::cout << "Opening UDP Port (Windows)" << std::endl;

    int result = 0;
    WSADATA wsaData;
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return false;
    }
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed reason: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &addr.sin_addr);

    m_clientSocks.push_back({sock, addr});
    m_socksOpen = true;
    return true;
}

bool screenCaptureWorkerWindows::closeUDPPorts() {
    if (m_socksOpen) {
        for (const auto& c : m_clientSocks) {
            std::cout << "Closing UDP Port (Windows): " << c.sock << std::endl;
            closesocket(c.sock);
            WSACleanup();
        }
    }
    m_socksOpen = false;
    m_clientSocks.clear();
    return true;
}

void screenCaptureWorkerWindows::runAnalFunc() {
    switch(m_conf.c_algo) {
    case ScreenLedAlgorithm::MEAN_DEFAULT:
        m_meanAlgo.analyzeColors(m_rgbData, m_conf, m_CommonPixelData);
        break;
    case ScreenLedAlgorithm::MEDIAN:
        m_medianAlgo.analyzeColors(m_rgbData, m_conf, m_CommonPixelData);
        break;
    }
}
