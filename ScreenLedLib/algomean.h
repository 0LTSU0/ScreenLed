#ifndef ALGOMEAN_H
#define ALGOMEAN_H

#include <memory>
#include <vector>
#include "Commons.h"

class AlgoMean
{
public:
    AlgoMean();

    void analyzeColors(std::vector<rgbValue>&, const ScreenCapConfig&, const RawPixelBuffer&);
};

#endif // ALGOMEAN_H
