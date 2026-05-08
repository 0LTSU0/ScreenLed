//#include "screenledgui.h"
#include "maingui.h"

#include <QApplication>
#include <QLibrary>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //ScreenLedGUI w;
    MainGUI w;
    w.show();
    return a.exec();
}
