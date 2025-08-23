#ifndef ALGOMEDIAN_H
#define ALGOMEDIAN_H

#include <memory>
#include <vector>
#include "Commons.h"

class AlgoMedian
{
public:
    AlgoMedian();
#ifdef _WIN32
    void analyzeColors(std::vector<rgbValue>&, const ScreenCapConfig&, const std::shared_ptr<DWORD[]>&);
#else
    void analyzeColors(std::vector<rgbValue>&, const ScreenCapConfig&, void*);
#endif
};

#endif // ALGOMEDIAN_H
