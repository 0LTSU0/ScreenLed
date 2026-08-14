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

void AlgoMedian::analyzeColors(std::vector<rgbValue>& res, const ScreenCapConfig& conf, const RawPixelBuffer& pixelData) {
    res.clear();
    res.assign(NUM_LED_SEGMENTS, rgbValue{});

    std::vector<std::vector<int>> reds(NUM_LED_SEGMENTS);
    std::vector<std::vector<int>> greens(NUM_LED_SEGMENTS);
    std::vector<std::vector<int>> blues(NUM_LED_SEGMENTS);

    int xPerSegment = conf.c_screenResX / NUM_LED_SEGMENTS;
    int res_y = pixelData.height;

    int starty, endy;
    switch(conf.c_analyzerScreenArea){
    case activeScreenArea::FULL:
        starty = 0;
        endy = res_y;
        break;
    case activeScreenArea::CENTER_THIRD:
        starty = res_y / 3;
        endy = starty * 2;
        break;
    case activeScreenArea::AUTO:
        //TODO
        starty = 0;
        endy = res_y;
        break;
    }

    for (int y = starty; y < endy; y += 10) {
        int currentSegment = 0;
        for (int x = 0; x < pixelData.width; x += 4) {
            if (x % xPerSegment == 0 && x != 0) currentSegment++;
            int r, g, b;
            pixelData.getPixel(x, y, r, g, b);
            reds[currentSegment].push_back(r);
            greens[currentSegment].push_back(g);
            blues[currentSegment].push_back(b);
        }
    }

    for (int i = 0; i < NUM_LED_SEGMENTS; ++i) {
        res[i].r = medianOf(reds[i]);
        res[i].g = medianOf(greens[i]);
        res[i].b = medianOf(blues[i]);
    }
}

