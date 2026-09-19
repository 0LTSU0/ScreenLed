#ifndef ALGOFLASHBOOST_H
#define ALGOFLASHBOOST_H

#include <vector>
#include "Commons.h"

class AlgoFlashBoost
{
public:
    AlgoFlashBoost();

    void analyzeColors(std::vector<rgbValue>&, const ScreenCapConfig&, const RawPixelBuffer&);

    std::vector<rgbValue> m_prevRgbVals;
    std::vector<int> m_framesSinceFlash;

private:
    int m_flashTreshold = 70;
    int m_dimmingSpeed = 10;

    std::vector<rgbValue> getRawMedian(const RawPixelBuffer&, const ScreenCapConfig&);
    int medianOf(std::vector<int>& arr);
    int getBrightness(const rgbValue&);
    rgbValue calcBrightnessForSegment(const rgbValue&, const int timeSinceFlash);
};

#endif // ALGOFLASHBOOST_H
