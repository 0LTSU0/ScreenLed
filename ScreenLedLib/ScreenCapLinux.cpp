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

    // ..so we need to find the primary display and find its position
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

void screenCaptureWorkerLinux::analyzeColors() {
    XImage* image = static_cast<XImage*>(m_image);
    if (!image || !image->data){
        return;
    }

    m_rgbData.clear();
    m_rgbData.assign(NUM_LED_SEGMENTS, rgbValue{});

    int xPerSegment = m_conf.c_screenResX / NUM_LED_SEGMENTS;
    int currentSegment = 0;
    int itemsPerCell = 0;
    for (int y = 0; y < image->height; y += 10) {
        currentSegment = 0;
        for (int x = 0; x < image->width; x += 4) {
            if (x % xPerSegment == 0 && x != 0) {
                currentSegment++;
            }
            if (currentSegment == 0) {
                itemsPerCell++;
            }

            unsigned long pixel = XGetPixel(image, x, y);
            int red   = (pixel & image->red_mask)   >> 16;
            int green = (pixel & image->green_mask) >> 8;
            int blue  = (pixel & image->blue_mask);

            m_rgbData[currentSegment].r += red;
            m_rgbData[currentSegment].g += green;
            m_rgbData[currentSegment].b += blue;
        }
    }

    for (int i = 0; i < NUM_LED_SEGMENTS; ++i) {
        m_rgbData[i].r /= itemsPerCell;
        m_rgbData[i].g /= itemsPerCell;
        m_rgbData[i].b /= itemsPerCell;
    }
}

void screenCaptureWorkerLinux::sendRGBData(const char* buf) {
    return;
}

bool screenCaptureWorkerLinux::openUDPPort() {
    return true;
}

bool screenCaptureWorkerLinux::closeUDPPort() {
    return true;
}
