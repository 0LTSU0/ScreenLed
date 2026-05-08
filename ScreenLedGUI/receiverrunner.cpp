#include "receiverrunner.h"
#include <qregularexpression.h>
#include <qversionnumber.h>

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

bool ReceiverRunner::findPythonInterpeter() {
#ifdef _WIN32
    QStringList candidates = {"py", "python3", "python"};
#else
    QStringList candidates = {"python3", "python"};
#endif

    QString newestPython;
    QVersionNumber newestVersion = QVersionNumber(0,0,0);

    for (const QString &cmd : candidates)
    {
        QProcess process;
        process.start(cmd, {"--version"});
        if (!process.waitForFinished(1000)) // 1s timeout
        {
            continue;
        }

        QString output = QString::fromLocal8Bit(process.readAllStandardOutput() + process.readAllStandardError()).trimmed();

        // Match version like "Python 3.13.3"
        QRegularExpression re("Python (\\d+)\\.(\\d+)\\.(\\d+)");
        QRegularExpressionMatch match = re.match(output);
        if (match.hasMatch())
        {
            int major = match.captured(1).toInt();
            int minor = match.captured(2).toInt();
            int patch = match.captured(3).toInt();
            QVersionNumber ver(major, minor, patch);

            if (ver > newestVersion)
            {
                newestVersion = ver;
                newestPython = cmd;
            }
        }
    }

    if (!newestPython.isEmpty())
    {
        qDebug() << "Newest python found from the system is: " << newestVersion.toString() << " using command: " << newestPython;
        m_pythonCmd = newestPython;
    }
    else
    {
        return false;
    }
    return true;
}
