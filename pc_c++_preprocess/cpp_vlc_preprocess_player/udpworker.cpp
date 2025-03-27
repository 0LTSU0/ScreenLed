#include "udpworker.h"
#include <fstream>

UDPWorker::UDPWorker() {}

void UDPWorker::udpLoop() {
    while (!udpLoopShouldStop) {
        qDebug() << "udpLoop() running:" << QThread::currentThread();
        QThread::sleep(1);
    }
}

void UDPWorker::stop() {
    udpLoopShouldStop = true;
}

bool UDPWorker::loadLedData(const QString &path) {
    if (path.isEmpty()) {
        return false;
    }

    std::ifstream file(path.toStdString().c_str(), std::ios::binary);
    if (!file) {
        return false;
    }

    unsigned char byte;
    bool numSegmentsRead = false;
    int color_ctr = 0;
    int seg_ctr = 0;
    int frame_ctr = 0;
    RGBData RGBval;
    while (file.get(reinterpret_cast<char&>(byte))) { // Read one byte at a time
        if (!numSegmentsRead) { //very first byte in the file tells us how many led segments we have
            m_numSegements = static_cast<int>(byte);
        }

        if (seg_ctr == 0) { //new segment begins
            switch (color_ctr) {
            case 0:
                RGBval.r = static_cast<int>(byte);
                break;
            case 1:
                RGBval.g = static_cast<int>(byte);
                break;
            case 2:
                RGBval.b = static_cast<int>(byte);
                break;
            default:
                break;
            }
            color_ctr++;
        }
        //std::cout << "Byte: " << static_cast<int>(byte) << std::endl; // Print as integer
    }

    return false;
}
