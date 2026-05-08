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
    explicit ReceiverRunner(const QString &starterScript, QObject *parent = nullptr)
        : m_scriptPath(starterScript) {}

    bool findPythonInterpeter();

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
    QString m_pythonCmd;
};

#endif // RECEIVERRUNNER_H
