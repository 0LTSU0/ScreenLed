#ifndef SCREENLEDGUI_H
#define SCREENLEDGUI_H

#include <QMainWindow>
#include <QThread>
#include <QTimer>

#include "consts.h"

typedef void (*MyPrototype)();
typedef float (*MyFloatPrototype)();

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

    // Function for interacting with screenled lib
    MyPrototype lib_startFunc;
    MyPrototype lib_endFunc;
    MyFloatPrototype lib_getFpsFunc;

private slots:
    void on_startButton_clicked();

    void on_saveConfig_clicked();

private:
    //functions
    int fillConfigForm();
    void saveConfigForm();
    void updateStatusLabel();

    //class variables
    Ui::ScreenLedGUI *ui;
    QThread screenLedTh;
    bool libScreenledThreadIsRunning = false;
    runStatus currentRunStatus = runStatus::IDLE;
    QTimer *statusUpdatetimer;  // Timer to schedule status label updates

    // TODO the config should probably be in some specific folder and be generated if it doesn't exist instead of hardocded path
    std::string gConfPath = "../pc_c++/gConfig.json";

};
#endif // SCREENLEDGUI_H
