#ifndef ALGOMEDIAN_H
#define ALGOMEDIAN_H

#include <memory>
#include <vector>
#include "Commons.h"

#ifdef _WIN32
// we only need Windows.h here but if we only include it, it breaks other includes in ScreenCapWindows :)
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>
#else
#endif

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
