#include "screenledgui.h"
#include "./ui_screenledgui.h"

ScreenLedGUI::ScreenLedGUI(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ScreenLedGUI)
{
    ui->setupUi(this);
}

ScreenLedGUI::~ScreenLedGUI()
{
    delete ui;
}
