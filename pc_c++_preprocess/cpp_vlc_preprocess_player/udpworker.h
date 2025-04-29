#ifndef UDPWORKER_H
#define UDPWORKER_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <vector>

struct RGBData {
    int r, g, b;
};

typedef std::vector<RGBData> frame;

class UDPWorker : public QObject
{
    Q_OBJECT

public slots:
    void udpLoop();

public:
    UDPWorker(QObject *parent = nullptr);
    void stop(); // can be called from outside for setting udpLoopShouldStop=true
    bool loadLedData(const QString &path);

private:
    volatile bool udpLoopShouldStop = false;
    int m_numSegements = 0;
    std::vector<frame> m_rgb_data; // vector with vector for each frame
};

#endif // UDPWORKER_H
