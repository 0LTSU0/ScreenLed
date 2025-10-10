#include "receiverrunner.h"

void ReceiverRunner::start()
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        qDebug() << "Process already running";
        return;
    }

    m_process = new QProcess(this);
    m_process->setProgram(m_pythonExc);
    m_process->setArguments({"-u", m_scriptPath});
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
        QByteArray output = m_process->readAllStandardOutput();
        emit outputReady(QString::fromLocal8Bit(output));
    });

    connect(m_process, &QProcess::finished, this, [this]() {
        emit finished();
    });

    m_process->start();
}


void ReceiverRunner::stop()
{
    if (!m_process || m_process->state() == QProcess::NotRunning)
    {
        qDebug() << "Process not running, skippin stop function";
        return;
    }

//#ifdef _WIN32
//    m_process->terminate();
//#else
//    ::kill(m_process->processId(), SIGINT);
//#endif

    m_process->write("stop\n");

    if (!m_process->waitForFinished(3000)) {
        // If process didn't quit gracefully, force kill even though this might leave clients alive
        m_process->kill();
        m_process->waitForFinished();
    }

    delete m_process;
    m_process = nullptr;
}
