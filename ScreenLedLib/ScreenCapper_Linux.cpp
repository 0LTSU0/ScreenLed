#include <ScreenCapper_Linux.h>
#include <QDebug>
#include <QApplication>
#include <QThread>
#include <immintrin.h>
#include <cstring>
#include <iostream>
#include <opencv2/opencv.hpp>
extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/XShm.h>   // now AFTER Xlib.h — Bool/Display/etc already defined
}
#include <sys/ipc.h>
#include <sys/shm.h>

cv::Mat xImageToMat(XImage* img) {
    int width = img->width;
    int height = img->height;

    cv::Mat mat(height, width, CV_8UC4, img->data);
    cv::Mat matBGR;
    cv::cvtColor(mat, matBGR, cv::COLOR_BGRA2BGR);

    return matBGR;
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

void ScreenCaptureWorker::updateConfig(ScreenCapConfig conf)
{
	std::lock_guard lock(m_confMutex);
	m_conf = conf;
}

void ScreenCaptureWorker::run()
{
    initScreenShotting();
    m_running = true;

    int loopctr = 0;
    auto fps_ctr_start = std::chrono::steady_clock::now();

    constexpr auto frame_time = std::chrono::microseconds(16667); // ~60 FPS

    while (m_running)
    {
        auto frame_start = std::chrono::steady_clock::now();

        screenshot();

        loopctr++;
        if (loopctr == 10)
        {
            auto now = std::chrono::steady_clock::now();
            double elapsed =
                std::chrono::duration<double>(now - fps_ctr_start).count();

            m_fps = 10.0 / elapsed;

            fps_ctr_start = now;
            loopctr = 0;
        }

        // Limit to ~60 FPS
        auto elapsed = std::chrono::steady_clock::now() - frame_start;

        if (elapsed < frame_time)
        {
            std::this_thread::sleep_for(frame_time - elapsed);
        }
    }
}

void ScreenCaptureWorker::stop()
{
	m_running = false;
	wait();
	deinitScreenShotting();
}

void ScreenCaptureWorker::initScreenShotting() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Failed to open X11 display!" << std::endl;
        return;
    }

    Window root = DefaultRootWindow(display);
    XRRScreenResources* screenResources = XRRGetScreenResources(display, root);
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

    m_shmSupported = XShmQueryExtension(display);
    if (!m_shmSupported) {
        std::cerr << "MIT-SHM not available, falling back to XGetImage (slower)" << std::endl;
    }

    m_shmWidth = 0;
    m_shmHeight = 0;
    m_shmInfoPtr = nullptr;
    m_image = nullptr;
}

void ScreenCaptureWorker::deinitScreenShotting() {
    if (m_display && m_image) {
        Display* display = static_cast<Display*>(m_display);
        if (m_shmSupported && m_shmInfoPtr) {
            XShmSegmentInfo* shmInfo = static_cast<XShmSegmentInfo*>(m_shmInfoPtr);
            XShmDetach(display, shmInfo);
            XDestroyImage(static_cast<XImage*>(m_image));
            shmdt(shmInfo->shmaddr);
            shmctl(shmInfo->shmid, IPC_RMID, nullptr);
            delete shmInfo;
            m_shmInfoPtr = nullptr;
        } else {
            XDestroyImage(static_cast<XImage*>(m_image));
        }
        m_image = nullptr;
    }

    if (m_display) {
        XCloseDisplay(static_cast<Display*>(m_display));
        m_display = nullptr;
    }

    std::lock_guard lock(m_confMutex);
    if (m_conf.c_showDebugPreview) {
        QThread::msleep(20);
        cv::destroyAllWindows();
    }
}


void ScreenCaptureWorker::screenshot() {
    if (!m_display) return;

    Display* display = static_cast<Display*>(m_display);
    Window root = static_cast<Window>(m_rootWindow);

    int res_x, res_y;
    bool showDebugPreview;
    {
        std::lock_guard lock(m_confMutex);
        res_x = m_conf.c_screenResX;
        res_y = m_conf.c_screenResY;
        showDebugPreview = m_conf.c_showDebugPreview;
    }

    if (m_shmSupported) {
        if (!m_image || res_x != m_shmWidth || res_y != m_shmHeight) {
            // tear down old SHM buffer if resolution changed
            if (m_image && m_shmInfoPtr) {
                XShmSegmentInfo* oldInfo = static_cast<XShmSegmentInfo*>(m_shmInfoPtr);
                XShmDetach(display, oldInfo);
                XDestroyImage(static_cast<XImage*>(m_image));
                shmdt(oldInfo->shmaddr);
                shmctl(oldInfo->shmid, IPC_RMID, nullptr);
                delete oldInfo;
                m_shmInfoPtr = nullptr;
                m_image = nullptr;
            }

            Visual* visual = DefaultVisual(display, DefaultScreen(display));
            int depth = DefaultDepth(display, DefaultScreen(display));

            XShmSegmentInfo* shmInfo = new XShmSegmentInfo();
            XImage* img = XShmCreateImage(display, visual, depth, ZPixmap,
                                           nullptr, shmInfo, res_x, res_y);
            if (!img) {
                std::cerr << "XShmCreateImage failed, disabling SHM" << std::endl;
                delete shmInfo;
                m_shmSupported = false;
            } else {
                shmInfo->shmid = shmget(IPC_PRIVATE,
                                         static_cast<size_t>(img->bytes_per_line) * img->height,
                                         IPC_CREAT | 0600);
                if (shmInfo->shmid < 0) {
                    std::cerr << "shmget failed, disabling SHM" << std::endl;
                    XDestroyImage(img);
                    delete shmInfo;
                    m_shmSupported = false;
                } else {
                    shmInfo->shmaddr = img->data = static_cast<char*>(shmat(shmInfo->shmid, nullptr, 0));
                    shmInfo->readOnly = False;

                    if (!XShmAttach(display, shmInfo)) {
                        std::cerr << "XShmAttach failed, disabling SHM" << std::endl;
                        shmdt(shmInfo->shmaddr);
                        shmctl(shmInfo->shmid, IPC_RMID, nullptr);
                        XDestroyImage(img);
                        delete shmInfo;
                        m_shmSupported = false;
                    } else {
                        m_image = img;
                        m_shmInfoPtr = shmInfo;
                        m_shmWidth = res_x;
                        m_shmHeight = res_y;
                    }
                }
            }
        }

        if (m_shmSupported) {
            XImage* img = static_cast<XImage*>(m_image);
            XShmSegmentInfo* shmInfo = static_cast<XShmSegmentInfo*>(m_shmInfoPtr);
            XShmGetImage(display, root, img, m_primaryDisplayOffsetX, m_primaryDisplayOffsetY, AllPlanes);
            (void)shmInfo; // silence unused warning if not otherwise referenced
        }
    }

    if (!m_shmSupported) {
        if (m_image) {
            XDestroyImage(static_cast<XImage*>(m_image));
            m_image = nullptr;
        }
        m_image = XGetImage(display, root, m_primaryDisplayOffsetX, m_primaryDisplayOffsetY,
                             res_x, res_y, AllPlanes, ZPixmap);
    }

    auto ss_time = std::chrono::steady_clock::now();
    convertToCommonSSFormat(ss_time);

    if (showDebugPreview) {
        cv::Mat mat = xImageToMat(static_cast<XImage*>(m_image));
        if (!mat.empty()) {
            cv::Mat preview = mat.clone();
            QMetaObject::invokeMethod(qApp, [preview]() {
                cv::imshow("Debug preview", preview);
                cv::waitKey(1);
            }, Qt::QueuedConnection);
        }
    }
}

void ScreenCaptureWorker::convertToCommonSSFormat(std::chrono::steady_clock::time_point ss_time)
{
    XImage* image = static_cast<XImage*>(m_image);
    if (!image || !image->data) return;

    int srcW;
	int srcH;
	int factor;
	{
		std::lock_guard lock(m_confMutex);
		srcW = m_conf.c_screenResX;
		srcH = m_conf.c_screenResY;
		factor = std::max(1, m_conf.c_analyzerDownscaleFactor);
	}
	const int outW = srcW / factor;
	const int outH = srcH / factor;
	
	if (outW <= 0 || outH <= 0) return;
	RawPixelBuffer CommonPixelData;
	CommonPixelData.rgb.resize(static_cast<size_t>(outW) * outH * 3);
	CommonPixelData.width = outW;
	CommonPixelData.height = outH;
	CommonPixelData.ss_timestamp = ss_time;

    if (image->bits_per_pixel == 32 &&
        image->byte_order == LSBFirst &&
        image->red_mask   == 0x00FF0000 &&
        image->green_mask == 0x0000FF00 &&
        image->blue_mask  == 0x000000FF)
    {
        const int stride = image->bytes_per_line;
        const unsigned char* src = reinterpret_cast<const unsigned char*>(image->data);
        unsigned char* dst = CommonPixelData.rgb.data();

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
        m_mailbox->put(CommonPixelData);
        return;
    }

    // Fallback unchanged — this one was already correctly strided
    for (int y = 0; y < outH; ++y) {
        for (int x = 0; x < outW; ++x) {
            unsigned long pixel = XGetPixel(image, x * factor, y * factor);
            unsigned char* p = &CommonPixelData.rgb[(static_cast<size_t>(y) * outW + x) * 3];
            p[0] = static_cast<unsigned char>((pixel & image->red_mask)   >> 16);
            p[1] = static_cast<unsigned char>((pixel & image->green_mask) >> 8);
            p[2] = static_cast<unsigned char>(pixel & image->blue_mask);
        }
    }

    m_mailbox->put(CommonPixelData);
}
