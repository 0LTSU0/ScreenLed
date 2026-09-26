#include "algoconfigwindow.h"
#include "ui_algoconfigwindow.h"

#include <QWidget>

AlgoConfigWindow::AlgoConfigWindow(QWidget *parent, QWidget *configWidget)
    : QDialog(parent)
    , ui(new Ui::AlgoConfigWindow)
{
    ui->setupUi(this);

    ui->AlgoConfigMainLayout->addWidget(configWidget);
}

AlgoConfigWindow::~AlgoConfigWindow()
{
    delete ui;
}
