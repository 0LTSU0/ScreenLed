#include "ScreenCapLinux.h"
#include <opencv2/opencv.hpp>
#include <QApplication>
#include <QThread>
extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>
}


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
}

void screenCaptureWorkerLinux::deinitScreenShotting() {
    if (m_image) {
        XDestroyImage(static_cast<XImage*>(m_image));
        m_image = nullptr;
    }

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

    int thirdY = m_conf.c_screenResY / 3;
    if (m_image) {
        XDestroyImage(static_cast<XImage*>(m_image));
        m_image = nullptr;
    }

    XImage* image = XGetImage(display, root, m_primaryDisplayOffsetX, m_primaryDisplayOffsetY + thirdY, m_conf.c_screenResX, thirdY, AllPlanes, ZPixmap);
    m_image = static_cast<void*>(image);

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

void screenCaptureWorkerLinux::sendRGBData(const char* buf) {
    ssize_t sent_bytes = sendto(m_sock, buf, strlen(buf), 0,
                                (sockaddr*)&m_outAddr, sizeof(m_outAddr));
    if (sent_bytes < 0) {
        std::cerr << "sendto() failed!" << std::endl;
    }
}

bool screenCaptureWorkerLinux::openUDPPort() {
    m_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_sock < 0) {
        perror("socket creation failed");
        return false;
    }
    m_outAddr.sin_family = AF_INET;
    m_outAddr.sin_port = htons(m_conf.c_raspiPort);
    inet_pton(AF_INET, m_conf.c_raspiIp.c_str(), &m_outAddr.sin_addr);
    return true;
}

bool screenCaptureWorkerLinux::closeUDPPort() {
    close(m_sock);
    return true;
}

void screenCaptureWorkerLinux::runAnalFunc() {
    switch(m_conf.c_algo) {
    case ScreenLedAlgorithm::MEAN_DEFAULT:
        m_meanAlgo.analyzeColors(m_rgbData, m_conf, m_image);
        break;
    case ScreenLedAlgorithm::MEDIAN:
        std::cout << "TODO" << std::endl;
        break;
    }
}
