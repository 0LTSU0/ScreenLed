#ifndef ALGOMEDIAN_H
#define ALGOMEDIAN_H

#include <memory>
#include <vector>
#include "Commons.h"

class AlgoMedian
{
public:
    AlgoMedian();

    void analyzeColors(std::vector<rgbValue>&, const ScreenCapConfig&, const RawPixelBuffer&);
};

#endif // ALGOMEDIAN_H
