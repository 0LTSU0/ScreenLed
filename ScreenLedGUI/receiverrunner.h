#ifndef RECEIVERRUNNER_H
#define RECEIVERRUNNER_H

#include <QObject>
#include <QThread>
#include <QProcess>
#include <QEventLoop>
#include <QDebug>

class ReceiverRunner : public QObject
{
    Q_OBJECT
public:
    explicit ReceiverRunner(const QString &starterScript, const QString &pythonExc,  QObject *parent = nullptr)
        : m_scriptPath(starterScript), m_pythonExc(pythonExc) {}

public slots:
    void start();
    void stop();

signals:
    void outputReady(const QString &line);
    void finished();

private:
    QString m_scriptPath;
    QString m_pythonExc;
    QProcess *m_process = nullptr;
};

#endif // RECEIVERRUNNER_H
