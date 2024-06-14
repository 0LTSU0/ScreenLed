#ifndef SCREENLEDGUI_H
#define SCREENLEDGUI_H

#include <QMainWindow>
#include <QThread>

typedef void (*MyPrototype)();

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

    // Function for starting screenled lib
    MyPrototype lib_startFunc;
    MyPrototype lib_endFunc;

private slots:
    void on_startButton_clicked();

private:
    //functions
    int fillConfigForm();
    void saveConfigForm();

    //class variables
    Ui::ScreenLedGUI *ui;
    QThread screenLedTh;
    bool threadIsRunning = false;

    // TODO the config should probably be in some specific folder and be generated if it doesn't exist instead of hardocded path
    std::string gConfPath = "../pc_c++/gConfig.json";

};
#endif // SCREENLEDGUI_H
