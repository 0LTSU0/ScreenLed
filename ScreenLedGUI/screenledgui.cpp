#include "screenledgui.h"
#include "./ui_screenledgui.h"
#include "json.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <QString>
using json = nlohmann::json;

ScreenLedGUI::ScreenLedGUI(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ScreenLedGUI)
{
    ui->setupUi(this);
    statusUpdatetimer = new QTimer(this);
    connect(statusUpdatetimer, &QTimer::timeout, this, &ScreenLedGUI::updateStatusLabel);
    statusUpdatetimer->start(1000);

    m_screenCapWorker->moveToThread(m_screenLibTh);
    QObject::connect(m_screenLibTh, &QThread::started, m_screenCapWorker, &screenCaptureWorkerWindows::run);
    fillConfigForm();
}

ScreenLedGUI::~ScreenLedGUI()
{
    if (m_libScreenledThreadIsRunning)
    {
        // Fake click to startButton callback to make sure thread exists before application is quit
        on_startButton_clicked();
    }
    delete ui;
}


int ScreenLedGUI::fillConfigForm()
{
    auto currentConfig = m_screenCapWorker->getCurrentConfig();

    // config file was found and it has expected keys -> try filling UI
    ui->debugSSVal->setCheckState(currentConfig.c_keepDebugSSOnClipboard ? Qt::Checked : Qt::Unchecked);
    ui->debugSsIntervalVal->setText(QString::number(currentConfig.c_debugSSInterval));
    ui->raspiPortVal->setText(QString::number(currentConfig.c_raspiPort));
    ui->raspiIpVal->setText(QString(currentConfig.c_raspiIp.c_str()));
    ui->showPreviewVal->setCheckState(currentConfig.c_showDebugPreview ? Qt::Checked : Qt::Unchecked);
    ui->screenResXval->setValue(currentConfig.c_screenResX);
    ui->screenResYval->setValue(currentConfig.c_screenResY);

    return 0;
}

void ScreenLedGUI::saveConfigForm()
{
    ScreenCapConfig newConf;
    bool convOk = true;

    newConf.c_raspiIp = ui->raspiIpVal->text().toStdString();
    int raspiPort = ui->raspiPortVal->text().toInt(&convOk);
    if (!convOk) {
        ui->raspiPortVal->clear(); // Todo show error message instead of just clearing it
        return;
    }
    newConf.c_raspiPort = raspiPort;
    int debugSsInterval = ui->debugSsIntervalVal->text().toInt(&convOk);
    if (!convOk)
    {
        ui->debugSsInterval->clear(); // Todo show error message instead of just clearing it
        return;
    }
    newConf.c_debugSSInterval = debugSsInterval;
    newConf.c_keepDebugSSOnClipboard = ui->debugSSVal->checkState() == Qt::Checked;
    newConf.c_showDebugPreview = ui->showPreviewVal->checkState() == Qt::Checked;
    newConf.c_screenResX = ui->screenResXval->value();
    newConf.c_screenResY = ui->screenResYval->value();

    m_screenCapWorker->updateCurrentConfig(newConf);
}

void ScreenLedGUI::updateStatusLabel()
{
    double curFPS = m_screenCapWorker->m_fps;
    QString status;
    status = "FPS: " + QString::number(curFPS) +", Status: ";
    switch (currentRunStatus) {
    case runStatus::IDLE:
        status += "IDLE";
        break;
    case runStatus::RUNNING:
        status += "RUNNING";
        break;
    default:
        status += "WTF??";
        break;
    }
    ui->StatusLabel->setText(status);
}

void ScreenLedGUI::on_startButton_clicked()
{
    if (!m_libScreenledThreadIsRunning)
    {
        saveConfigForm();
        m_screenLibTh->start();
        m_libScreenledThreadIsRunning = true;
        ui->startButton->setText("Stop");
        currentRunStatus = runStatus::RUNNING;
    }
    else
    {
        m_screenCapWorker->stop();
        m_screenLibTh->quit();
        m_screenLibTh->wait();
        ui->startButton->setText("Start");
        m_libScreenledThreadIsRunning = false;
        currentRunStatus = runStatus::IDLE;
    }
}

void ScreenLedGUI::on_saveConfig_clicked()
{
    saveConfigForm();
}

