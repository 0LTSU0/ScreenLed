#include "algomean.h"

AlgoMean::AlgoMean() {}

void AlgoMean::analyzeColors(std::vector<rgbValue>& res, const ScreenCapConfig& conf, const RawPixelBuffer& pixelData) {
    res.clear();
    res.assign(NUM_LED_SEGMENTS, rgbValue{});
    int res_x = pixelData.width;
    int res_y = pixelData.height;
    int xPerSegment = res_x / NUM_LED_SEGMENTS;

    int starty, endy;
    switch(conf.c_analyzerScreenArea){
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

    int currentSegment = 0;
    std::vector<int> itemsPerSegment(NUM_LED_SEGMENTS, 0); // track each segment's count separately

    for (int yi = starty; yi < endy; yi += 10) { //loop every 10th line
        for (int x = 0; x < res_x; x += 4) { //loop every 4th column
            currentSegment = std::min(x / xPerSegment, NUM_LED_SEGMENTS - 1);

            itemsPerSegment[currentSegment]++;

            int r, g, b;
            pixelData.getPixel(x, yi, r, g, b);
            res[currentSegment].r += r;
            res[currentSegment].g += g;
            res[currentSegment].b += b;
        }
    }

    //get average based on how many values were used for calculating total
    for (int i = 0; i < NUM_LED_SEGMENTS; ++i) {
        if (itemsPerSegment[i] > 0) {
            res[i].r /= itemsPerSegment[i];
            res[i].g /= itemsPerSegment[i];
            res[i].b /= itemsPerSegment[i];
        }
    }
}

