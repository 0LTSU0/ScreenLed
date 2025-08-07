#include "ScreenCapLinux.h"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}


void screenCaptureWorkerLinux::initScreenShotting() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Failed to open X11 display!" << std::endl;
        return;
    }

    Window root = DefaultRootWindow(display);
    XWindowAttributes gwa;
    XGetWindowAttributes(display, root, &gwa);

    m_display = static_cast<void*>(display);
    m_rootWindow = root;
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

    XImage* image = XGetImage(display, root, 0, thirdY, m_conf.c_screenResY, thirdY, AllPlanes, ZPixmap);
    m_image = static_cast<void*>(image);
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
