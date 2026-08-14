#include "algomean.h"

AlgoMean::AlgoMean() {}

void AlgoMean::analyzeColors(std::vector<rgbValue>& res, const ScreenCapConfig& conf, const RawPixelBuffer& pixelData) {
    res.clear();
    res.assign(NUM_LED_SEGMENTS, rgbValue{});
    int res_x = conf.c_screenResX;
    int res_y = conf.c_screenResY;
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
    int itemsPerCell = 0;
    for (int yi = starty; yi < endy; yi += 10) { //loop every 10th line
        currentSegment = 0;
        for (int x = 0; x < res_x; x += 4) { //loop every 4th row WARNING: IF RESOLUTION IS VERY WEIRD THIS WILL MISBEHAVE
            if (x % xPerSegment == 0 && x != 0) { //in every line the row will correspond to all segments
                currentSegment++;
            }
            if (currentSegment == 0) { //track how many items has been put per cell
                itemsPerCell++;
            }

            // Get the pixel value at (x, yi)
            int r, g, b;
            pixelData.getPixel(x, yi, r, g, b);

            res[currentSegment].r += r;
            res[currentSegment].g += g;
            res[currentSegment].b += b;
        }
    }

    //get average based on how many values were used for calculating total
    for (int i=0; i<NUM_LED_SEGMENTS; i++) {
        res[i].r /= itemsPerCell;
        res[i].g /= itemsPerCell;
        res[i].b /= itemsPerCell;
    }
}

