#include <algoflashboost.h>

#include <algorithm>
#include <iostream>

AlgoFlashBoost::AlgoFlashBoost() 
{
	m_prevRgbVals.resize(NUM_LED_SEGMENTS);
	m_framesSinceFlash.assign(NUM_LED_SEGMENTS, 0);
};

void AlgoFlashBoost::analyzeColors(std::vector<rgbValue>& result, const ScreenCapConfig& conf, const RawPixelBuffer& pixelData)
{
    result.clear();
    result.assign(NUM_LED_SEGMENTS, rgbValue{});
    auto rawMedians = getRawMedian(pixelData, conf);

    if (rawMedians.size() != m_prevRgbVals.size())
    {
        std::cout << "WTF rawMedians size is different than m_prevRgbVals" << std::endl;
        result.assign(NUM_LED_SEGMENTS, rgbValue{});
        return;
    }

    for (int i = 0; i < rawMedians.size(); i++)
    {
        auto brightnessNow = getBrightness(rawMedians.at(i));
        auto brightnessBefore = getBrightness(m_prevRgbVals.at(i));
        if (brightnessNow - brightnessBefore > m_flashTreshold)
        {
            std::cout << "flash on segment " << i << std::endl;
            m_framesSinceFlash.at(i) = 0;   
        }
        else if (brightnessBefore - brightnessNow > m_flashTreshold)
        {
            std::cout << "DEflash on segment" << i << std::endl;
            m_framesSinceFlash.at(i) = 255; // no matter what 255 frames is large enough fake time for the calculator to choose the raw rgb value
        }
        result.at(i) = calcBrightnessForSegment(rawMedians.at(i), m_framesSinceFlash.at(i));
        m_framesSinceFlash.at(i) += 1;
    }

    m_prevRgbVals = rawMedians;
}

int AlgoFlashBoost::medianOf(std::vector<int>& arr) {
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

int AlgoFlashBoost::getBrightness(const rgbValue& rgb)
{
    return (rgb.r + rgb.g + rgb.b) / 3;
}

rgbValue AlgoFlashBoost::calcBrightnessForSegment(const rgbValue& medianRGB, int timeSinceFlash)
{
    int maxChannel = std::max({ medianRGB.r, medianRGB.g, medianRGB.b });
    if (maxChannel == 0)
    {
        return medianRGB;
    }
    
    double maxerscale = 255.0 / maxChannel;
    rgbValue maxBrightnessed;
    maxBrightnessed.r = std::max(static_cast<int>(medianRGB.r * maxerscale) - m_dimmingSpeed * timeSinceFlash, medianRGB.r);
    maxBrightnessed.g = std::max(static_cast<int>(medianRGB.g * maxerscale) - m_dimmingSpeed * timeSinceFlash, medianRGB.g);
    maxBrightnessed.b = std::max(static_cast<int>(medianRGB.b * maxerscale) - m_dimmingSpeed * timeSinceFlash, medianRGB.b);

    return maxBrightnessed;
}

std::vector<rgbValue> AlgoFlashBoost::getRawMedian(const RawPixelBuffer& pixelData, const ScreenCapConfig& conf)
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