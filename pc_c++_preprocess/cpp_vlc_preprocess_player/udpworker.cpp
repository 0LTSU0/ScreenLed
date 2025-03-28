#include "udpworker.h"
#include <fstream>
#include <iostream>

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

bool readFrame(int num_segments, std::vector<RGBData> &frame, std::ifstream &file)
{
    int bit_counter = 0;
    int cc = 0;
    int bits_per_frame = num_segments * 3;
    unsigned char byte;
    RGBData rgbval;
    while(file.get(reinterpret_cast<char&>(byte)))
    {
        if (bit_counter == bits_per_frame)
        {
            file.unget(); // because of the loop we have read one too maby bytes this iteration -> put the last one back to the stream
            return true;
        }
        if (cc == 3) // every three bits we have completed one rgb value
        {
            cc = 0;
        }

        switch(cc)
        {
        case 0:
            rgbval.r = static_cast<int>(byte);
            break;
        case 1:
            rgbval.g = static_cast<int>(byte);
            break;
        case 2:
            rgbval.b = static_cast<int>(byte);
            frame.push_back(rgbval);
            break;
        }
        cc++;
        bit_counter++;
    }

    return false; // there wasn't enough bits to read
}

bool UDPWorker::loadLedData(const QString &path) {
    m_rgb_data.clear();
    if (path.isEmpty()) {
        return false;
    }
    std::ifstream file(path.toStdString().c_str(), std::ios::binary);
    if (!file) {
        return false;
    }
    unsigned char byte;
    frame cur_frame;
    file.get(reinterpret_cast<char&>(byte)); //first byte in the file should tell us how many segments each frame has
    int num_segments = static_cast<int>(byte);

    while(readFrame(num_segments, cur_frame, file))
    {
        m_rgb_data.push_back(cur_frame);
        cur_frame.clear();
    }

    return true;
}
