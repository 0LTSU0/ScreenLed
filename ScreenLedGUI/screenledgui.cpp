#include "screenledgui.h"
#include "./ui_screenledgui.h"
#include <fstream>
#include <iostream>
#include <string>
#include <QString>

ScreenLedGUI::ScreenLedGUI(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ScreenLedGUI)
{
    ui->setupUi(this);
    statusUpdatetimer = new QTimer(this);
    connect(statusUpdatetimer, &QTimer::timeout, this, &ScreenLedGUI::updateStatusLabel);
    statusUpdatetimer->start(1000);

    m_screenCapWorker->moveToThread(m_screenLibTh);
#if defined(WIN32)
    QObject::connect(m_screenLibTh, &QThread::started, m_screenCapWorker, &screenCaptureWorkerWindows::run);
    ui->showPreviewVal->setEnabled(false); // on windows we don't support showing preview output TODO: ADD SUPPORT :)
#else
    QObject::connect(m_screenLibTh, &QThread::started, m_screenCapWorker, &screenCaptureWorkerLinux::run);
    ui->debugSSVal->setEnabled(false); // on linux we don't support putting image to clipboard because it seems unnecessarily difficult to implement
#endif
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
    ui->showPreviewVal->setCheckState(currentConfig.c_showDebugPreview ? Qt::Checked : Qt::Unchecked);
    ui->screenResXval->setValue(currentConfig.c_screenResX);
    ui->screenResYval->setValue(currentConfig.c_screenResY);

    int activeIndex = 0;
    int i = 0;
    for (const auto& val : algoNameMap) {
        ui->algoSelectVal->addItem(QString::fromStdString(val.first));
        if (val.second == currentConfig.c_algo) {
            activeIndex = i;
        }
        i++;
    }
    ui->algoSelectVal->setCurrentIndex(activeIndex);

    bool firstClientSet = false; // GUI has one client fields which need special handling
    for (auto &client : currentConfig.c_clientInfos) {
        if (!firstClientSet) {
            ui->raspiIpVal1->setText(QString::fromStdString(client.host));
            ui->raspiPortVal1->setText(QString::number(client.port));
            firstClientSet = true;
        } else {
            addClientRowToGUI(client.host, client.port); // rest need to be added as "extra" rows
        }
    }


    return 0;
}

void ScreenLedGUI::saveConfigForm()
{
    ScreenCapConfig newConf;
    bool convOk = true;
    
    if (m_extraClientIpInputs.size() != m_extraClientPortInputs.size()) {
        std::cerr << "m_extraClientIpInputs and m_extraClientPortInputs somehow have different sizes -> cannot save config!" << std::endl;
    }

    newConf.c_clientInfos.clear(); // default value needs to be removed from the vector
    newConf.c_clientInfos.push_back({ui->raspiIpVal1->text().toStdString(), ui->raspiPortVal1->text().toInt(&convOk)}); //TODO: check convOk for these
    for (int i=0; i < m_extraClientPortInputs.size(); i++) { // Above check ensures that both vectors have the same size -> size of either one can be used here
        newConf.c_clientInfos.push_back({m_extraClientIpInputs.at(i)->text().toStdString(), m_extraClientPortInputs.at(i)->text().toInt(&convOk)});
    }

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
    newConf.c_algo = algoNameMap[ui->algoSelectVal->currentText().toStdString()];

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

void ScreenLedGUI::addClientRowToGUI(const std::string ip = "", int port = -1) {
    m_numRaspis++;
    std::string txt = "Raspi" + std::to_string(m_numRaspis) + " IP:PORT";
    QHBoxLayout *row = new QHBoxLayout;
    QSpacerItem *leftSpacer  = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QLabel *label            = new QLabel(QString::fromStdString(txt));
    QLineEdit *edit1         = new QLineEdit;
    QLineEdit *edit2         = new QLineEdit;
    QSpacerItem *rightSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    label->setStyleSheet("color: rgb(255, 255, 255);");
    edit1->setStyleSheet("background-color: rgb(153, 153, 153);");
    edit2->setStyleSheet("background-color: rgb(153, 153, 153);");
    if (!ip.empty()) {
        edit1->setText(QString::fromStdString(ip));
    }
    if (port != -1) {
        edit2->setText(QString::number(port));
    }
    row->addItem(leftSpacer);
    row->addWidget(label);
    row->addWidget(edit1);
    row->addWidget(edit2);
    row->addItem(rightSpacer);
    row->setStretch(0, 1);
    row->setStretch(1, 2);
    row->setStretch(2, 2);
    row->setStretch(3, 2);
    row->setStretch(4, 1);
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(centralWidget()->layout());
    if (mainLayout) {
        mainLayout->insertLayout(3 + m_numRaspis - 2, row);
        m_extraClientIpInputs.push_back(edit1);
        m_extraClientPortInputs.push_back(edit2);
    }

}

void ScreenLedGUI::on_saveConfig_clicked()
{
    saveConfigForm();
}


void ScreenLedGUI::on_algoSelectVal_currentTextChanged(const QString &arg1)
{
    m_screenCapWorker->m_conf.c_algo = algoNameMap[arg1.toStdString()]; //TODO: might be dangerous to change this while the application runs if its read at the same time
}

//TODO: NEED TO MAKE A DELETE BUTTON
void ScreenLedGUI::on_addAnotherClientButt_clicked()
{
    addClientRowToGUI();
}

