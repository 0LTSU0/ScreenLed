#include "preprocessplayer.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PreProcessPlayer w;

    // Don't proceed if some init failed when window was created
    if (!w.get_vlc_init_success())
    {
        return 1;
    }

    w.show();
    return a.exec();
}
