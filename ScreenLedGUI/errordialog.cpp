#include "errordialog.h"
#include "ui_errordialog.h"

ErrorDialog::ErrorDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ErrorDialog)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowModality(Qt::ApplicationModal);
}

ErrorDialog::~ErrorDialog()
{
    delete ui;
}

void ErrorDialog::Error(QString err)
{
    setWindowTitle("Error");
    ui->errorMsg->setText(err);
    show();
}

void ErrorDialog::Warning(QString err)
{
    setWindowTitle("Warning");
    ui->errorMsg->setText(err);
    show();
}
