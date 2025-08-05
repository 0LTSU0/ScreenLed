#include "screenledgui.h"

#include <QApplication>
#include <QLibrary>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ScreenLedGUI w;
    w.show();
    return a.exec();
}
