#include "algomedian.h"
#include <algorithm>

AlgoMedian::AlgoMedian() {}

auto medianOf = [](std::vector<int>& arr) -> int {
    if (arr.empty()) return 0;
    std::sort(arr.begin(), arr.end());
    size_t n = arr.size();
    if (n % 2 == 1) {
        return arr[n / 2];
    } else {
        return (arr[n / 2 - 1] + arr[n / 2]) / 2;
    }
};

#ifdef _WIN32
void AlgoMedian::analyzeColors(std::vector<rgbValue>& res, const ScreenCapConfig& conf, const std::shared_ptr<DWORD[]>& pixelData) {
    res.clear();
    res.assign(NUM_LED_SEGMENTS, rgbValue{});

    std::vector<std::vector<int>> reds(NUM_LED_SEGMENTS);
    std::vector<std::vector<int>> greens(NUM_LED_SEGMENTS);
    std::vector<std::vector<int>> blues(NUM_LED_SEGMENTS);

    int xPerSegment = conf.c_screenResX / NUM_LED_SEGMENTS;
    int res_y = conf.c_screenResY / 3;
    int currentSegment = 0;
    DWORD pixelValue;
    for (int yi = 0; yi < res_y; yi += 10) {
        currentSegment = 0;
        for (int x = 0; x < conf.c_screenResX; x += 4) {
            if (x % xPerSegment == 0 && x != 0) {
                currentSegment++;
            }

            // Get the pixel value at (x, yi)
            pixelValue = pixelData[yi * conf.c_screenResX + x];

            // Extract the red, green, and blue components
            int blue = static_cast<int>(GetRValue(pixelValue));
            int green = static_cast<int>(GetGValue(pixelValue));
            int red = static_cast<int>(GetBValue(pixelValue));

            reds[currentSegment].push_back(red);
            greens[currentSegment].push_back(green);
            blues[currentSegment].push_back(blue);
        }
    }

    for (int i = 0; i < NUM_LED_SEGMENTS; ++i) {
        res[i].r = medianOf(reds[i]);
        res[i].g = medianOf(greens[i]);
        res[i].b = medianOf(blues[i]);
    }
}

#else
// Linux implementation for analyzeColors() of this algo
extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>
}
void AlgoMedian::analyzeColors(std::vector<rgbValue>& res, const ScreenCapConfig& conf, void* img) {
    XImage* image = static_cast<XImage*>(img);
    if (!image || !image->data){
        return;
    }

    res.clear();
    res.assign(NUM_LED_SEGMENTS, rgbValue{});

    std::vector<std::vector<int>> reds(NUM_LED_SEGMENTS);
    std::vector<std::vector<int>> greens(NUM_LED_SEGMENTS);
    std::vector<std::vector<int>> blues(NUM_LED_SEGMENTS);

    int xPerSegment = conf.c_screenResX / NUM_LED_SEGMENTS;
    int currentSegment = 0;
    for (int y = 0; y < image->height; y += 10) {
        currentSegment = 0;
        for (int x = 0; x < image->width; x += 4) {
            if (x % xPerSegment == 0 && x != 0) {
                currentSegment++;
            }

            unsigned long pixel = XGetPixel(image, x, y);
            int red   = (pixel & image->red_mask)   >> 16;
            int green = (pixel & image->green_mask) >> 8;
            int blue  = (pixel & image->blue_mask);

            reds[currentSegment].push_back(red);
            greens[currentSegment].push_back(green);
            blues[currentSegment].push_back(blue);
        }
    }

    for (int i = 0; i < NUM_LED_SEGMENTS; ++i) {
        res[i].r = medianOf(reds[i]);
        res[i].g = medianOf(greens[i]);
        res[i].b = medianOf(blues[i]);
    }
}
#endif
