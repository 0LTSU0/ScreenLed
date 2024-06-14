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
    typedef void (*MyPrototype)();
    if (slLIB.load()) {
        MyPrototype lib_startFunc = (MyPrototype) slLIB.resolve("libStartFunction");
        if (!lib_startFunc) {
            qWarning("Could not resolve function: %s", slLIB.errorString().toStdString().c_str());
        }
    } else {
        qWarning("Could not load library: %s", slLIB.errorString().toStdString().c_str());
    }



    return a.exec();
}
