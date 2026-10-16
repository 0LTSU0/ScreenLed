#pragma once

#include <QDebug>
#include <QLoggingCategory>
#include <QString>

Q_DECLARE_LOGGING_CATEGORY(guiLog)
Q_DECLARE_LOGGING_CATEGORY(libLog)

namespace Log
{
    // Call once at application startup.
    void init(const QString &filePath);

    // Configure independent output thresholds.
    void setConsoleLevel(QtMsgType level);
    void setFileLevel(QtMsgType level);
}

#define GUI_DEBUG()   qCDebug(guiLog)
#define GUI_INFO()    qCInfo(guiLog)
#define GUI_WARN()    qCWarning(guiLog)
#define GUI_ERROR()   qCCritical(guiLog)

#define LIB_DEBUG()  qCDebug(libLog)
#define LIB_INFO()   qCInfo(libLog)
#define LIB_WARN()   qCWarning(libLog)
#define LIB_ERROR()  qCCritical(libLog)

