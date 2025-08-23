#include "algomean.h"

AlgoMean::AlgoMean() {}

#ifdef _WIN32
// Windows implementation for analyzeColors() of this algo
void AlgoMean::analyzeColors(std::vector<rgbValue>& res, const ScreenCapConfig& conf, const std::shared_ptr<DWORD[]>& pixelData) {
    res.clear();
    res.assign(NUM_LED_SEGMENTS, rgbValue{});
    int res_x = conf.c_screenResX;
    int res_y = conf.c_screenResY / 3;
    int xPerSegment = res_x / NUM_LED_SEGMENTS;

    int currentSegment = 0;
    int itemsPerCell = 0;
    DWORD pixelValue;
    for (int yi = 0; yi < res_y; yi += 10) { //loop every 10th line
        currentSegment = 0;
        for (int x = 0; x < res_x; x += 4) { //loop every 4th row WARNING: IF RESOLUTION IS VERY WEIRD THIS WILL MISBEHAVE
            if (x % xPerSegment == 0 && x != 0) { //in every line the row will correspond to all segments
                currentSegment++;
            }
            if (currentSegment == 0) { //track how many items has been put per cell
                itemsPerCell++;
            }

            // Get the pixel value at (x, yi)
            pixelValue = pixelData[yi * res_x + x];

            // Extract the red, green, and blue components
            int blue = static_cast<int>(GetRValue(pixelValue));
            int green = static_cast<int>(GetGValue(pixelValue));
            int red = static_cast<int>(GetBValue(pixelValue));

            res[currentSegment].r += red;
            res[currentSegment].g += green;
            res[currentSegment].b += blue;
        }
    }

    //get average based on how many values were used for calculating total
    for (int i=0; i<NUM_LED_SEGMENTS; i++) {
        res[i].r /= itemsPerCell;
        res[i].g /= itemsPerCell;
        res[i].b /= itemsPerCell;
    }
}
#else
// Linux implementation for analyzeColors() of this algo
void AlgoMean::analyzeColors(rgbValue& res) {

}
#endif
