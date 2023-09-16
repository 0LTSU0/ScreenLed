#include "boost/asio.hpp"
#include <iostream>
#include <Windows.h>
#include <chrono>

using namespace std::chrono;
using namespace boost::asio;
using ip::tcp;


#define RES_X 1920
#define RES_Y 1080
#define LED_SEGMENTS 20
#define KEEP_SS_ON_CLIPBOARD false
#define RASPI_IP "192.168.1.69"
//#define RASPI_IP "127.0.0.1"
#define RASI_PORT 65432


void takeScreenShot(DWORD* pixelData, int y) {
    HDC hScreenDC = GetDC(nullptr);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, RES_X, y);
    HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(hMemoryDC, hBitmap));
    BitBlt(hMemoryDC, 0, 0, RES_X, y, hScreenDC, 0, y, SRCCOPY);
    hBitmap = static_cast<HBITMAP>(SelectObject(hMemoryDC, hOldBitmap));

    //Enable to get the screenshots to clipboard for debugging
#if KEEP_SS_ON_CLIPBOARD
    OpenClipboard(NULL);
    EmptyClipboard();
    SetClipboardData(CF_BITMAP, hBitmap);
    CloseClipboard();
#endif

    // Put pixel data to pixelData
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = RES_X;
    bmi.bmiHeader.biHeight = y; // Negative height for a top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32; // Assuming 32-bit color depth
    GetDIBits(hMemoryDC, hBitmap, 0, y, pixelData, &bmi, DIB_RGB_COLORS);


    //Cleanup
    SelectObject(hMemoryDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(nullptr, hScreenDC);
}


void analyze(DWORD* pixelData, int y, int** result) {
    int xInSegment = RES_X / LED_SEGMENTS;

    //sample = [segment][sumr,sumG,sumB]
    int** sample = new int* [LED_SEGMENTS];
    for (int i = 0; i < LED_SEGMENTS; i++) {
        sample[i] = new int[3];
        for (int j = 0; j < 3; j++) {
            sample[i][j] = 0;
        }
    }

    //get colors
    int currentSegment = 0;
    int itemsPerCell = 0;
    for (int yi = 0; yi < y; yi += 10) { //loop every 10th line
        currentSegment = 0;
        for (int x = 0; x < RES_X; x += 4) { //loop every 4th row
            if (x % xInSegment == 0 && x != 0) { //in every line the row will correspond to all segments
                currentSegment++;
            }
            if (currentSegment == 0) { //track how many items has been put per cell
                itemsPerCell++;
            }

            // Get the pixel value at (x, yi)
            DWORD pixelValue = pixelData[yi * RES_X + x];

            // Extract the red, green, and blue components
            int blue = static_cast<int>(GetRValue(pixelValue));
            int green = static_cast<int>(GetGValue(pixelValue));
            int red = static_cast<int>(GetBValue(pixelValue));

            sample[currentSegment][0] += red;
            sample[currentSegment][1] += green;
            sample[currentSegment][2] += blue;
        }
    }


    for (int i = 0; i < LED_SEGMENTS; i++) {
        for (int j = 0; j < 3; j++) {
            result[i][j] = sample[i][j] / itemsPerCell;
        }
    }

    //for (int i = 0; i < LED_SEGMENTS; i++) {
    //    std::cout << "Segment " << i << ": " << result[i][0] << "," << result[i][1] << "," << result[i][2] << std::endl;
    //}

    for (int i = 0; i < LED_SEGMENTS; i++) {
        delete[] sample[i];
    }
    delete[] sample;
}


//Generate string that has all the rgb values in it
std::string generateMessage(int** result) {
    std::string msg{ "" };

    for (int i = 0; i < LED_SEGMENTS; i++) {
        for (int j = 0; j < 3; j++) {
            msg += std::to_string(result[i][j]);
            msg += ",";
        }
        msg += ";";
    }

    return msg;
}


int main() {
    std::cout << "Starting" << std::endl;

    // y = RES_Y -> fullscreen, y = RES_Y/3 -> middle third
    const int y = RES_Y / 3;

    // Variable for pixel data
    DWORD* pixelData = new DWORD[RES_X * y];

    // Result
    int** result = new int* [LED_SEGMENTS];
    for (int i = 0; i < LED_SEGMENTS; i++) {
        result[i] = new int[3];
    }

    //socket
    boost::asio::io_service io_service;
    tcp::socket socket(io_service);
    socket.connect(tcp::endpoint(boost::asio::ip::address::from_string(RASPI_IP), RASI_PORT));
    boost::system::error_code error;

    auto prev_ts = std::chrono::system_clock::now();
    int ctr = 0;
    while (true) {
        takeScreenShot(pixelData, y);
        analyze(pixelData, y, result);
        ctr++;

        //Print result (debugging purposes)
        //for (int i = 0; i < LED_SEGMENTS; i++) {
        //    std::cout << "Segment " << i << ": " << result[i][0] << "," << result[i][1] << "," << result[i][2] << std::endl;
        //}

        //send result
        std::string sendData = generateMessage(result);
        boost::asio::write(socket, boost::asio::buffer(sendData), error);

        //fps
        std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - prev_ts;
        prev_ts = std::chrono::system_clock::now();
        if (ctr == 10) {
            std::cout << "\r" << "Current FPS:" << 1 / elapsed_seconds.count() << std::flush;
            ctr = 0;
        }

    }

    delete[] pixelData;
    return 0;
}