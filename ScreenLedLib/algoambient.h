#ifndef ALGOAMBIENT_H
#define ALGOAMBIENT_H

#include <vector>
#include "Commons.h"

class AlgoAmbient
{
public:
    AlgoAmbient() {};

    void analyzeColors(std::vector<rgbValue>&, const ScreenCapConfig&, const RawPixelBuffer&);

private:
    int medianOf(std::vector<int>&);
    std::vector<rgbValue> getRawMedian(const RawPixelBuffer& pixelData, const ScreenCapConfig& conf);
};

#endif // ALGOAMBIENT_H
