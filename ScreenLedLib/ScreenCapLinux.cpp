#include "ScreenCapLinux.h"
#include <opencv2/opencv.hpp>
#include <QApplication>
#include <QThread>
#include <chrono>
#include <QDebug>
extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>
}
#include <immintrin.h>
#include <cstring>

cv::Mat xImageToMat(XImage* img) {
    int width = img->width;
    int height = img->height;

    cv::Mat mat(height, width, CV_8UC4, img->data);
    cv::Mat matBGR;
    cv::cvtColor(mat, matBGR, cv::COLOR_BGRA2BGR);

    return matBGR;
}

void screenCaptureWorkerLinux::initScreenShotting() {
    // The window created here contains all displays and the resolution will be the sum of them..
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Failed to open X11 display!" << std::endl;
        return;
    }

    // ..so we need to find the position of our primary display
    Window root = DefaultRootWindow(display);
    XRRScreenResources* screenResources = XRRGetScreenResources(display, root);
    bool foundPrimaryDisplay = false;
    RROutput primaryOutput = XRRGetOutputPrimary(display, root);
    XRROutputInfo* outputInfo = XRRGetOutputInfo(display, screenResources, primaryOutput);
    XRRCrtcInfo* crtcInfo = XRRGetCrtcInfo(display, screenResources, outputInfo->crtc);

    m_display = static_cast<void*>(display);
    m_rootWindow = root;
    m_primaryDisplayOffsetX = crtcInfo->x;
    m_primaryDisplayOffsetY = crtcInfo->y;

    XRRFreeCrtcInfo(crtcInfo);
    XRRFreeOutputInfo(outputInfo);
    XRRFreeScreenResources(screenResources);

    m_CommonPixelData.width = m_conf.c_screenResX;
    m_CommonPixelData.height = m_conf.c_screenResY;
    m_CommonPixelData.rgb.resize(static_cast<size_t>(m_CommonPixelData.width) * m_CommonPixelData.height * 3);
}

void screenCaptureWorkerLinux::deinitScreenShotting() {
    //if (m_image) {
    //    XDestroyImage(static_cast<XImage*>(m_image));
    //    m_image = nullptr;
    //}

    if (m_display) {
        XCloseDisplay(static_cast<Display*>(m_display));
        m_display = nullptr;
    }

    if (m_conf.c_showDebugPreview) {
        QThread::msleep(20);
        cv::destroyAllWindows();
    }
}

void screenCaptureWorkerLinux::takeScreenShot() {
    if (!m_display) return;

    Display* display = static_cast<Display*>(m_display);
    Window root = static_cast<Window>(m_rootWindow);

    if (m_image) {
        XDestroyImage(static_cast<XImage*>(m_image));
        m_image = nullptr;
    }

    XImage* image = XGetImage(display, root, m_primaryDisplayOffsetX, m_primaryDisplayOffsetY, m_conf.c_screenResX, m_conf.c_screenResY, AllPlanes, ZPixmap);
    m_image = static_cast<void*>(image);
    //auto start = std::chrono::high_resolution_clock::now();
    convertToCommonSSFormat(image);
    //auto end = std::chrono::high_resolution_clock::now();
    //double ms = std::chrono::duration<double, std::milli>(end - start).count();
    //qDebug() << "convertToCommonSSFormat took" << ms << "ms";

    // cv::imshow only works in main thread unless this nonsense is done
    // also this has MAJOR drop in performance and should really only
    // be enabled when trying to debug the output
    if (m_conf.c_showDebugPreview) {
        cv::Mat mat = xImageToMat(image);
        if (!mat.empty()) {
            cv::Mat preview = mat.clone();
            QMetaObject::invokeMethod(qApp, [preview]() {
                cv::imshow("Debug preview", preview);
                cv::waitKey(1);
            }, Qt::QueuedConnection);
        }
    }
}

static void convertRowBGRXtoRGB_SSE(const unsigned char* src, unsigned char* dst, int width)
{
    const __m128i mask = _mm_setr_epi8(
        2,1,0, 6,5,4, 10,9,8, 14,13,12, -1,-1,-1,-1);

    int x = 0;
    for (; x + 4 <= width; x += 4) {
        __m128i pix = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src + x * 4));
        __m128i shuffled = _mm_shuffle_epi8(pix, mask);
        std::memcpy(dst + x * 3, &shuffled, 12); // only 12 valid bytes, safe to write exactly that many
    }
    for (; x < width; ++x) { // tail for widths not divisible by 4
        dst[x*3+0] = src[x*4+2];
        dst[x*3+1] = src[x*4+1];
        dst[x*3+2] = src[x*4+0];
    }
}

void screenCaptureWorkerLinux::convertToCommonSSFormat(void* ximage)
{
    XImage* image = static_cast<XImage*>(ximage);
    if (!image || !image->data) return;

    const int factor = std::max(1, m_conf.c_analyzerDownscaleFactor);
    const int outW = image->width  / factor;
    const int outH = image->height / factor;
    if (outW <= 0 || outH <= 0) return;

    if (m_CommonPixelData.width != outW || m_CommonPixelData.height != outH) {
        m_CommonPixelData.width  = outW;
        m_CommonPixelData.height = outH;
        m_CommonPixelData.rgb.resize(static_cast<size_t>(outW) * outH * 3);
    }

    if (image->bits_per_pixel == 32 &&
        image->byte_order == LSBFirst &&
        image->red_mask   == 0x00FF0000 &&
        image->green_mask == 0x0000FF00 &&
        image->blue_mask  == 0x000000FF)
    {
        const int stride = image->bytes_per_line;
        const unsigned char* src = reinterpret_cast<const unsigned char*>(image->data);
        unsigned char* dst = m_CommonPixelData.rgb.data();

        for (int y = 0; y < outH; ++y) {
            const unsigned char* srcRow = src + static_cast<size_t>(y * factor) * stride;
            unsigned char* dstRow = dst + static_cast<size_t>(y) * outW * 3;

            if (factor == 1) {
                // contiguous row -> SIMD shuffle applies correctly
                convertRowBGRXtoRGB_SSE(srcRow, dstRow, outW);
            } else {
                // strided sampling -> scalar (gather isn't a win here)
                for (int x = 0; x < outW; ++x) {
                    const unsigned char* p = srcRow + (static_cast<size_t>(x) * factor) * 4;
                    dstRow[x*3+0] = p[2]; // R
                    dstRow[x*3+1] = p[1]; // G
                    dstRow[x*3+2] = p[0]; // B
                }
            }
        }
        return;
    }

    // Fallback unchanged — this one was already correctly strided
    for (int y = 0; y < outH; ++y) {
        for (int x = 0; x < outW; ++x) {
            unsigned long pixel = XGetPixel(image, x * factor, y * factor);
            unsigned char* p = &m_CommonPixelData.rgb[(static_cast<size_t>(y) * outW + x) * 3];
            p[0] = static_cast<unsigned char>((pixel & image->red_mask)   >> 16);
            p[1] = static_cast<unsigned char>((pixel & image->green_mask) >> 8);
            p[2] = static_cast<unsigned char>(pixel & image->blue_mask);
        }
    }
}

void screenCaptureWorkerLinux::sendRGBData(const char* buf) {
    for (const auto& c : m_clientSocks) {
        ssize_t sent_bytes = sendto(c.sock, buf, strlen(buf), 0,
                                    (sockaddr*)&c.addr, sizeof(c.addr));
        if (sent_bytes < 0) {
            std::cerr << "sendto() failed!" << std::endl;
        }
    }
}

bool screenCaptureWorkerLinux::openUDPPort(const char* host, int port) {
    auto sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket creation failed");
        return false;
    }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &addr.sin_addr);
    m_clientSocks.push_back({sock, addr});
    return true;
}

bool screenCaptureWorkerLinux::closeUDPPorts() {
    if (!m_clientSocks.empty()) {
        for (const auto& c : m_clientSocks) {
            close(c.sock);
        }
    }
    m_clientSocks.clear();
    return true;
}

void screenCaptureWorkerLinux::runAnalFunc() {
    switch(m_conf.c_algo) {
    case ScreenLedAlgorithm::MEAN_DEFAULT:
        m_meanAlgo.analyzeColors(m_rgbData, m_conf, m_CommonPixelData);
        break;
    case ScreenLedAlgorithm::MEDIAN:
        m_medianAlgo.analyzeColors(m_rgbData, m_conf, m_CommonPixelData);
        break;
    }
}
