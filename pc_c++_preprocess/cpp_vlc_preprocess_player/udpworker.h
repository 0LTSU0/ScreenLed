#ifndef UDPWORKER_H
#define UDPWORKER_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <vector>

struct RGBData {
    int r, g, b;
};

class UDPWorker : public QObject
{
    Q_OBJECT

public slots:
    void udpLoop();

public:
    UDPWorker();
    void stop(); // can be called from outside for setting udpLoopShouldStop=true
    bool loadLedData(const QString &path);

private:
    volatile bool udpLoopShouldStop = false;
    int m_numSegements = 0;
    std::vector<std::vector<RGBData>> m_ledData; // vector with vector for each frame. The inner vector has rgb datas for for that frame
};

#endif // UDPWORKER_H
