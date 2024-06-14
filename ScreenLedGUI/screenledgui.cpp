#include "screenledgui.h"
#include "./ui_screenledgui.h"
#include "json.hpp"
#include <fstream>
#include <iostream>
#include <string>
using json = nlohmann::json;

ScreenLedGUI::ScreenLedGUI(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ScreenLedGUI)
{
    ui->setupUi(this);

    fillConfigForm();
}

ScreenLedGUI::~ScreenLedGUI()
{
    delete ui;
}


int ScreenLedGUI::fillConfigForm()
{
    std::ifstream ifs(gConfPath);
    if (ifs.fail())
    {
        exit(1); // TODO should actually generate the config with default values instead of quitting
    }

    json conf = json::parse(ifs);
    if ( !(conf.contains("raspiPort") && conf.contains("raspiIp") && conf.contains("keepDebugSS") && conf.contains("debugSSInterval")) ) {
        std::cout << "Cannot start application: gConfig.json is malformed!" << std::endl;
        exit(1); // TODO should should actually fix the file instead of quitting
    }

    // config file was found and it has expected keys -> try filling UI
    ui->debugSSVal->setCheckState(conf["keepDebugSS"].get<bool>() ? Qt::Checked : Qt::Unchecked);
    ui->debugSsIntervalVal->setText(QString::number(conf["debugSSInterval"].get<int>()));
    ui->raspiPortVal->setText(QString::number(conf["raspiPort"].get<int>()));
    ui->raspiIpVal->setText(QString(conf["raspiIp"].get<std::string>().c_str()));

    ifs.close();
    return 0;
}

void ScreenLedGUI::saveConfigForm()
{
    json updatedConf;
    bool convOk = true;
    updatedConf["raspiIp"] = ui->raspiIpVal->text().toStdString();
    int raspiPort = ui->raspiPortVal->text().toInt(&convOk);
    if (!convOk)
    {
        return; //TODO: Show Error
    }
    updatedConf["raspiPort"] = raspiPort;
    int debugSsInterval = ui->debugSsIntervalVal->text().toInt(&convOk);
    if (!convOk)
    {
        return; //TODO: Show Error
    }
    updatedConf["debugSSInterval"] = debugSsInterval;
    updatedConf["keepDebugSS"] = ui->debugSSVal->checkState() == Qt::Checked;

    std::ofstream file(gConfPath);
    file << updatedConf.dump(4);
    file.close();
}

void ScreenLedGUI::on_startButton_clicked()
{
    if (!threadIsRunning)
    {
        saveConfigForm();
        QObject::connect(&screenLedTh, &QThread::started, [&]() {
            qDebug() << "screenLedLibrary startup";
            lib_startFunc();
            qDebug() << "screenLedLibrary thread quit";
        });
        screenLedTh.start();
        threadIsRunning = true;
        ui->startButton->setText("Stop");
    }
    else
    {
        lib_endFunc();
        screenLedTh.quit();
        screenLedTh.wait();
        ui->startButton->setText("Start");
        threadIsRunning = false;
    }
}

