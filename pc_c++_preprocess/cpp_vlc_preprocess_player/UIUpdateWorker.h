#ifndef UIUPDATEWORKER_H
#define UIUPDATEWORKER_H

#include <QObject>
#include <QDebug>
#include <QThread>

class UIUpdateWorker : public QObject
{
    Q_OBJECT

public:
    bool volatile m_stopped = false;

public slots:
    void updateUILoop() {
        while (!m_stopped) {  // Example: Infinite loop (replace with real work)
            qDebug() << "Worker running in thread:" << QThread::currentThread();
            QThread::sleep(1);  // Simulate work
        }
        qDebug() << "Broke out of infinite UDP loop";
    }

    void stop() {
        m_stopped = true;
    }
};

#endif // UIUPDATEWORKER_H
