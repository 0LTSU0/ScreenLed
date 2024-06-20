#include "screenledgui.h"

#include <QApplication>
#include <QLibrary>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ScreenLedGUI w;
    w.show();

    // Load screenLed library
    QLibrary slLIB("lib/ScreenLedLIB");

    if (slLIB.load()) {
        w.lib_startFunc = (MyPrototype) slLIB.resolve("libStartFunction");
        w.lib_endFunc = (MyPrototype) slLIB.resolve("libStopFunction");
        w.lib_getFpsFunc = (MyFloatPrototype) slLIB.resolve("libGetCurrentFPS");
        if (!w.lib_startFunc || !w.lib_endFunc || !w.lib_getFpsFunc) {
            qWarning("Could not resolve function: %s", slLIB.errorString().toStdString().c_str());
        }
    } else {
        qWarning("Could not load library: %s", slLIB.errorString().toStdString().c_str());
    }

    return a.exec();
}
