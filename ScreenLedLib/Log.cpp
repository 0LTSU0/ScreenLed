#include "Log.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>

#include <cstdio>
#include <cstdlib>

Q_LOGGING_CATEGORY(guiLog, "screenled.gui")
Q_LOGGING_CATEGORY(libLog, "screenled.lib")

namespace
{
    QMutex logMutex;
    QFile logFile;

    QtMsgType consoleLevel = QtInfoMsg;
    QtMsgType fileLevel = QtDebugMsg;

    constexpr qint64 maxFileSize = 5 * 1024 * 1024; // 5 MiB
    constexpr int backupCount = 3;

    int severity(QtMsgType type)
    {
        switch (type) {
        case QtDebugMsg:    return 0;
        case QtInfoMsg:     return 1;
        case QtWarningMsg:  return 2;
        case QtCriticalMsg: return 3;
        case QtFatalMsg:    return 4;
        }
        return 0;
    }

    bool shouldLog(QtMsgType type, QtMsgType minimum)
    {
        return severity(type) >= severity(minimum);
    }

    QString levelName(QtMsgType type)
    {
        switch (type) {
        case QtDebugMsg:    return QStringLiteral("DEBUG");
        case QtInfoMsg:     return QStringLiteral("INFO ");
        case QtWarningMsg:  return QStringLiteral("WARN ");
        case QtCriticalMsg: return QStringLiteral("ERROR");
        case QtFatalMsg:    return QStringLiteral("FATAL");
        }
        return QStringLiteral("?????");
    }

    QString componentName(const char *category)
    {
        const QString name = QString::fromUtf8(
            category ? category : "default");

        if (name == QStringLiteral("screenled.gui"))
            return QStringLiteral("GUI");

        if (name.startsWith(QStringLiteral("screenled.gui.")))
            return QStringLiteral("GUI/") + name.mid(8);

        if (name == QStringLiteral("screenled.lib"))
            return QStringLiteral("CORE");

        if (name.startsWith(QStringLiteral("screenled.lib.")))
            return QStringLiteral("CORE/") + name.mid(9);

        // Keep other categories visible, including ordinary qDebug().
        return name;
    }

    void writeConsole(const QByteArray &line)
    {
        std::fwrite(line.constData(), 1,
                    static_cast<size_t>(line.size()), stderr);
        std::fflush(stderr);
    }

    bool openLogFile()
    {
        if (logFile.isOpen())
            logFile.close();

        return logFile.open(QIODevice::WriteOnly |
                            QIODevice::Append);
    }

    void rotateLogFile()
    {
        if (!logFile.isOpen() || logFile.size() < maxFileSize)
            return;

        const QString path = logFile.fileName();
        logFile.close();

        // Remove the oldest backup.
        QFile::remove(path + QStringLiteral(".%1").arg(backupCount));

        // Shift .1 -> .2, .2 -> .3, and so on.
        for (int i = backupCount - 1; i >= 1; --i) {
            const QString from =
                path + QStringLiteral(".%1").arg(i);
            const QString to =
                path + QStringLiteral(".%1").arg(i + 1);

            if (QFile::exists(from))
                QFile::rename(from, to);
        }

        // Move the active log to .1.
        if (QFile::exists(path) && !QFile::rename(path, path + ".1")) {
            writeConsole(
                "Logging: could not rotate log file; appending instead\n");
        }

        if (!openLogFile()) {
            writeConsole(
                "Logging: could not reopen log file after rotation\n");
        }
    }

    void messageHandler(QtMsgType type,
                        const QMessageLogContext &context,
                        const QString &message)
    {
        QMutexLocker locker(&logMutex);

        const QString timestamp =
            QDateTime::currentDateTime().toString(
                QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz"));

        const QString component = componentName(context.category);
        const QString source = QString::fromUtf8(
            context.file ? context.file : "?");

        const QString line = QStringLiteral("[%1] [%2] [%3] %4:%5 %6\n")
            .arg(timestamp)
            .arg(levelName(type))
            .arg(component)
            .arg(source)
            .arg(context.line)
            .arg(message);

        const QByteArray bytes = line.toUtf8();

        if (shouldLog(type, consoleLevel))
            writeConsole(bytes);

        if (shouldLog(type, fileLevel)) {
            if (logFile.isOpen()) {
                rotateLogFile();

                if (logFile.isOpen()) {
                    logFile.write(bytes);
                    logFile.flush();
                }
            }
        }

        // Qt will terminate the process after a fatal message handler
        // returns. Do not log recursively or call qFatal() here.
    }
}

namespace Log
{
    void init(const QString &filePath)
    {
        QMutexLocker locker(&logMutex);

        const QFileInfo info(filePath);
        const QString directory = info.absolutePath();

        if (!QDir().mkpath(directory)) {
            writeConsole(
                "Logging: could not create log directory\n");
        }

        logFile.setFileName(filePath);

        if (!openLogFile()) {
            writeConsole(
                "Logging: could not open log file; file logging disabled\n");
        }

        qInstallMessageHandler(messageHandler);
    }

    void setConsoleLevel(QtMsgType level)
    {
        QMutexLocker locker(&logMutex);
        consoleLevel = level;
    }

    void setFileLevel(QtMsgType level)
    {
        QMutexLocker locker(&logMutex);
        fileLevel = level;
    }
}

