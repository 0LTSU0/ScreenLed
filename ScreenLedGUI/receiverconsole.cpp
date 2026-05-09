#include "receiverconsole.h"
#include "ui_receiverconsole.h"

ReceiverConsole::ReceiverConsole(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ReceiverConsole)
{
    ui->setupUi(this);
}

ReceiverConsole::~ReceiverConsole()
{
    delete ui;
}

void ReceiverConsole::on_closeButton_clicked()
{
    close();
}

void ReceiverConsole::appendOutput(QString str) {
    ui->receiverOutput->appendPlainText(str);
}


void ReceiverConsole::fillInitialOutput(std::vector<QString> &strs) {
    for (auto& str : strs) {
        appendOutput(str);
    }
}
