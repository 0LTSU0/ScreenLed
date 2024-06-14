#ifndef SCREENLEDGUI_H
#define SCREENLEDGUI_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class ScreenLedGUI;
}
QT_END_NAMESPACE

class ScreenLedGUI : public QMainWindow
{
    Q_OBJECT

public:
    ScreenLedGUI(QWidget *parent = nullptr);
    ~ScreenLedGUI();

private:
    //class variables


    //functions
    Ui::ScreenLedGUI *ui;
};
#endif // SCREENLEDGUI_H
