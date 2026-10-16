#include "algoambient.h"
#include "Log.h"

void AlgoAmbient::analyzeColors(std::vector<rgbValue>& result, const ScreenCapConfig& conf, const RawPixelBuffer& pixelData)
{
    result.clear();
    result.assign(NUM_LED_SEGMENTS, rgbValue{});

    auto rawMedians = getRawMedian(pixelData, conf);
    result = rawMedians;
}

int AlgoAmbient::medianOf(std::vector<int>& arr) {
    if (arr.empty()) return 0;

    std::sort(arr.begin(), arr.end());

    size_t n = arr.size();

    if (n % 2 == 1) {
        return arr[n / 2];
    }
    else {
        return (arr[n / 2 - 1] + arr[n / 2]) / 2;
    }
}

std::vector<rgbValue> AlgoAmbient::getRawMedian(const RawPixelBuffer& pixelData, const ScreenCapConfig& conf)
{
    auto res = std::vector<rgbValue>();
    res.assign(NUM_LED_SEGMENTS, rgbValue{});

    std::vector<std::vector<int>> reds(NUM_LED_SEGMENTS);
    std::vector<std::vector<int>> greens(NUM_LED_SEGMENTS);
    std::vector<std::vector<int>> blues(NUM_LED_SEGMENTS);

    int xPerSegment = pixelData.width / NUM_LED_SEGMENTS;
    int res_y = pixelData.height;

    int starty = 0;
    int endy = 0;
    switch (conf.c_analyzerScreenArea) {
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
        for (int x = 0; x < pixelData.width; x += 4) {
            int currentSegment = std::min(x / xPerSegment, NUM_LED_SEGMENTS - 1);
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

    return res;
}
