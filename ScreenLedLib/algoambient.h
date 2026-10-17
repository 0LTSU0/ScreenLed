#ifndef ALGOAMBIENT_H
#define ALGOAMBIENT_H

#include <vector>
#include "Commons.h"
#include <chrono>

class AlgoAmbient
{
public:
    AlgoAmbient() {
        m_prevMedians.assign(NUM_LED_SEGMENTS, rgbValue{});
    };

    void analyzeColors(std::vector<rgbValue>&, const ScreenCapConfig&, const RawPixelBuffer&);

private:
    int medianOf(std::vector<int>&);
    std::vector<rgbValue> getRawMedian(const RawPixelBuffer& pixelData, const ScreenCapConfig& conf);
    std::vector<rgbValue> getResult(const ScreenCapConfig& conf);

    std::vector<rgbValue> m_prevMedians;
    std::vector<rgbValue> m_currentMedians;
    std::chrono::steady_clock::time_point m_prevMedianTimeStamp = std::chrono::steady_clock::now();

};

#endif // ALGOAMBIENT_H
