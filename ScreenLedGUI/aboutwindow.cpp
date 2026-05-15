#include "aboutwindow.h"
#include "ui_aboutwindow.h"
#include "version.h"

AboutWindow::AboutWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AboutWindow)
{
    ui->setupUi(this);
    ui->versionText->setText(
        QString("ScreenLedGUI version %1").arg(APP_VERSION)
    );
}

AboutWindow::~AboutWindow()
{
    delete ui;
}

void AboutWindow::on_versionPopupOkButt_clicked()
{
    this->close();
}

