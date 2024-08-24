#include "defs.h"
#include "configurator.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <vector>
#include <algorithm>

#if IS_WINDOWS
//some windows specific stuff?
#else
#include "clk.h"
#include "gpio.h"
#include "dma.h"
#include "pwm.h"
#include "ws2811.h"
ws2811_t ledstring =
{
    .freq = WS2811_TARGET_FREQ,
    .dmanum = 10,
    .channel =
    {
        [0] =
        {
            .gpionum = 18,
            .invert = 0,
            .count = 280,
            .strip_type = WS2812_STRIP,
            .brightness = 255,
        },
        [1] =
        {
            .gpionum = 0,
            .invert = 0,
            .count = 0,
            .brightness = 0,
        },
    },
};
#endif

static void analyze_colors(cv::Mat &frame, cv::VideoCapture &cap, std::vector<int> *resArr)
{
    int resx = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int resy = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    int seg_width = resx / NUM_SEGMENTS;
    int center_third_start = resy / 3;
    cv::Scalar avg_color;
    for (int i = 0; i < NUM_SEGMENTS; i++)
    {
        //Rect_ (_Tp _x, _Tp _y, _Tp _width, _Tp _height)
        cv::Rect segment_rect(i*seg_width, center_third_start, seg_width, center_third_start);
        cv::Mat segment = frame(segment_rect);
        avg_color = cv::mean(segment);
        resArr->push_back(avg_color[2]);
        resArr->push_back(avg_color[1]);
        resArr->push_back(avg_color[0]);
    }
}

void print_colors(std::vector<int> &arr)
{
    std::string str = "";
    int ctr = 0;
    for (int& val : arr)
    {
        std::stringstream ss;
        ss << val << ",";
        str.append(ss.str());
    }
    std::cout << str << std::endl;
}

#if !IS_WINDOWS
void set_strip(std::vector<int>& arr)
{
    for (int i = 0; i < NUM_SEGMENTS; i++)
    {
        int r = std::max(arr[3 * i + 0] - 10, 0);
        int g = std::max(arr[3 * i + 1] - 10, 0);
        int b = std::max(arr[3 * i + 2] - 20, 0);
        int w = 0; //white is always zero
        uint32_t color = (w << 24) | (r << 16) | (g << 8) | b;
        
        //std::stringstream stream;
        //stream << std::hex << color;
        //std::string result(stream.str());
        //std::cout << "Setting led to " << result << std::endl;

        for (int mp = 0; mp < NUM_LEDS_PER_SEG; mp++)
        {
            ledstring.channel[0].leds[i*NUM_LEDS_PER_SEG + mp] = color;
        }

    }
    ws2811_render(&ledstring);
}
#endif

int main(int argc, char *argv[])
{
    std::cout << "starting..." << std::endl;

    if (argc != 2) 
    {
        std::cout << "Startup failed: Please give path to configuration file as argument!" << std::endl;
        return 1;
    }

    //Read config and create ledstrings based on that. The led library can handle only max two separate strips!
    c_configurator conf;
    if (!conf.readConfigJSON(argv[1]))
    {
        return 1; //configurator should print some kind of error message
    }
    
    static cv::VideoCapture cap(0);

    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open capture card." << std::endl;
        return -1;
    }

#if !IS_WINDOWS
    ws2811_return_t ret;
    if ((ret = ws2811_init(&ledstring)) != WS2811_SUCCESS)
    {
        std::cout << "ws2811_init failed: " << ws2811_get_return_t_str(ret);
        return ret;
    }
#endif

    // Set camera properties
    cap.set(cv::CAP_PROP_FRAME_WIDTH, conf.configuration.camera_res_h);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, conf.configuration.camera_res_v);
    cap.set(cv::CAP_PROP_FPS, conf.configuration.camera_target_fps);

    cv::Mat frame;
    std::vector<int> colorArray; //filled with r1,b1,g1,r2,b2,g2,r3,b3,...
    std::vector<float> timeArray;
    while (true) {
        auto start = std::chrono::system_clock::now();

        // Capture a new frame from the camera
        cap >> frame;

        // Check if the frame was captured correctly
        if (frame.empty()) {
            std::cerr << "Error: Could not grab a frame :C" << std::endl;
            break;
        }

        // Display the frame (if windows)
        if (SHOW_PREVIEW)
        {
            cv::imshow("Capture feed", frame);
        }

        colorArray.clear();
        analyze_colors(frame, cap, &colorArray);

        //print colors if windows or set to rgb if linux
#if IS_WINDOWS
        print_colors(colorArray);
#else
        set_strip(colorArray);
#endif
        
        // Break the loop if the user presses the 'q' key
        if (cv::waitKey(1) == 'q') {
            break;
        }

        std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - start;
        timeArray.push_back(static_cast<float>(elapsed_seconds.count()));
        if (timeArray.size() == 60)
        {
            float sumFPS = 0.0;
            for (float& val : timeArray)
            {
                sumFPS = sumFPS + val;
            }
            std::cout << "\r FPS from average of " << timeArray.size() << " loops: " << 1.0 / (sumFPS / timeArray.size()) << std::flush;
            timeArray.clear();
        }
        
    }


    // Release the camera and Destroy any open windows
    cap.release();
    cv::destroyAllWindows();

    return 0;
}
