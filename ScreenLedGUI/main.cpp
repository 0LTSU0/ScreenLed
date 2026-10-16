//#include "screenledgui.h"
#include "maingui.h"

#include <QApplication>
#include <QLibrary>
#include <QStandardPaths>
#include "Log.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Init logger
    QCoreApplication::setApplicationName("ScreenLed");
    const QString logPath =
        QStandardPaths::writableLocation(
            QStandardPaths::AppLocalDataLocation)
        + "/logs/screenled.log";
    Log::init(logPath);
    GUI_INFO() << "ScreenLed starting";

    MainGUI w;
    w.show();
    return a.exec();
}
