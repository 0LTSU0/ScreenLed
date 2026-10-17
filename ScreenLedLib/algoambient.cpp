#include "algoambient.h"
#include "Log.h"
#include <algorithm>

void AlgoAmbient::analyzeColors(std::vector<rgbValue>& result, const ScreenCapConfig& conf, const RawPixelBuffer& pixelData)
{
    result.clear();
    result.assign(NUM_LED_SEGMENTS, rgbValue{});

    auto rawMedians = getRawMedian(pixelData, conf);
    auto timeNow = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(timeNow - m_prevMedianTimeStamp).count() >= conf.c_algoAmbientConfig.c_sampleSeconds)
    {
        LIB_DEBUG() << "Prev medians are older than " << conf.c_algoAmbientConfig.c_sampleSeconds << "s";
        m_prevMedians = m_currentMedians;
        m_prevMedianTimeStamp = timeNow;
        m_currentMedians = rawMedians;
    }

    result = getResult(conf);
}

std::vector<rgbValue> AlgoAmbient::getResult(const ScreenCapConfig& conf)
{
    std::vector<rgbValue> res;
    if (m_prevMedians.empty() || m_currentMedians.empty())
    {
        LIB_DEBUG() << "AlgoAmbient: prev or current rawmedians is empty. Cannot calculate result";
        res.assign(NUM_LED_SEGMENTS, rgbValue{});
        return res;
    }

    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - m_prevMedianTimeStamp).count();
    double t = std::clamp(elapsed / conf.c_algoAmbientConfig.c_sampleSeconds, 0.0, 1.0);
    LIB_DEBUG() << "t=" << t;
    for (int i=0; i<NUM_LED_SEGMENTS; i++)
    {
        auto diffR = m_currentMedians.at(i).r - m_prevMedians.at(i).r;
        auto diffG = m_currentMedians.at(i).g - m_prevMedians.at(i).g;
        auto diffB = m_currentMedians.at(i).b - m_prevMedians.at(i).b;

        auto rgb = rgbValue{};
        rgb.r = static_cast<int>(std::lround(m_prevMedians.at(i).r + diffR * t));
        rgb.g = static_cast<int>(std::lround(m_prevMedians.at(i).g + diffG * t));
        rgb.b = static_cast<int>(std::lround(m_prevMedians.at(i).b + diffB * t));
        res.push_back(rgb);
    }
    return res;
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
