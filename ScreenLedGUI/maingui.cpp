#include "maingui.h"
#include "ui_maingui.h"
#include "aboutwindow.h"
#include "settingswindow.h"
#include "errordialog.h"

#include <QApplication>
#include <QString>
#include <QMessageBox>
#include <QFile>

MainGUI::MainGUI(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainGUI)
{
    ui->setupUi(this);
    ui->statusbar->showMessage("IDLE");

#if defined(WIN32)
    m_screenCapWorker = new screenCaptureWorkerWindows(m_gConfPath);
#else
    m_screenCapWorker = new screenCaptureWorkerLinux(m_gConfPath);
#endif
    m_screenCapWorker->moveToThread(m_screenLibTh);
#if defined(WIN32)
    QObject::connect(m_screenLibTh, &QThread::started, m_screenCapWorker, &screenCaptureWorkerWindows::run);
#else
    QObject::connect(m_screenLibTh, &QThread::started, m_screenCapWorker, &screenCaptureWorkerLinux::run);
#endif

    populateAlgoSelect();
    populateReceiverStatusRows();

    connect(m_uiUpdateTimer, &QTimer::timeout, this, &MainGUI::periodicUIUpdate);
    m_uiUpdateTimer->start(1000);
}

MainGUI::~MainGUI()
{
    onExitActions();
    delete ui;
}

void MainGUI::populateAlgoSelect() {
    auto currentConfig = m_screenCapWorker->getCurrentConfig();
    int activeIndex = 0;
    int i = 0;
    for (const auto& val : algoNameMap) {
        ui->mainGUIAlgoSelect->addItem(QString::fromStdString(val.first));
        if (val.second == currentConfig.c_algo) {
            activeIndex = i;
        }
        i++;
    }
    ui->mainGUIAlgoSelect->setCurrentIndex(activeIndex);
}

void MainGUI::populateReceiverStatusRows() {
    if (!m_receiverStatusRows.empty()) {
        for (auto row : m_receiverStatusRows) {
            QLayoutItem *item;
            while ((item = row->takeAt(0)) != nullptr) {
                if (item->widget()) delete item->widget();
                delete item;
            }
            ui->receiverStatusContainer->removeItem(row);
            delete row;
        }
        m_receiverStatusRows.clear();
    }

    for (auto client : m_screenCapWorker->getCurrentConfig().c_clientInfos) {
        QString statString = "Status N/A for this type";
        if (client.type == receiverType::RASPI_SSH) {
            statString = "Not running";
        }

        QString hostStr = QString("%1:%2").arg(QString::fromStdString(client.host)).arg(client.port);

        QHBoxLayout *rowLayout = new QHBoxLayout();
        rowLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
        rowLayout->addWidget(new QLabel(QString::fromStdString(receiverTypeValueMap[client.type])));
        rowLayout->addWidget(new QLabel(hostStr));
        rowLayout->addWidget(new QLabel(statString));
        rowLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

        ui->receiverStatusContainer->addLayout(rowLayout);
        m_receiverStatusRows.append(rowLayout);
    }
}

void MainGUI::updateReceiverStatusRow(QString host, QString status)
{
    // this is bad and very brittle. Should store something more sensible in m_receiverStatusRows than the rowLayouts
    for (auto& rowLayout : m_receiverStatusRows)
    {
        if (qobject_cast<QLabel*>(rowLayout->itemAt(2)->widget())->text().contains(host))
        {
            qobject_cast<QLabel*>(rowLayout->itemAt(3)->widget())->setText(status);
            return;
        }
    }
}

void MainGUI::updateAllSSHReceiverStatusRows(QString status)
{
    // this is bad and very brittle. Should store something more sensible in m_receiverStatusRows than the rowLayouts
    for (auto& rowLayout : m_receiverStatusRows)
    {
        if (qobject_cast<QLabel*>(rowLayout->itemAt(1)->widget())->text().contains("ssh"))
        {
            qobject_cast<QLabel*>(rowLayout->itemAt(3)->widget())->setText(status);
        }
    }
}

void MainGUI::onExitActions() {
    if (m_libRunStatus == runStatus::RUNNING) {
        on_startButt_clicked(); // if still running when exiting, stop the library first
    }

    // Make sure currently selected algo gets saved since its on the main window rather than configuration and thus has no save button
    m_screenCapWorker->updateCurrentConfig(m_screenCapWorker->getCurrentConfig());

    stopReceivers();
}

void MainGUI::on_actionExit_triggered()
{
    onExitActions();
    QApplication::quit();
}


void MainGUI::on_mainGUIAlgoSelect_currentTextChanged(const QString &arg1)
{
    for (const auto& val : algoNameMap) {
        if (val.first == arg1.toStdString()) {
            m_screenCapWorker->getCurrentConfig().c_algo = val.second;
            return;
        }
    }
    qDebug() << "WTF, the selected option in mainguialgoselect does not match any existing algotype";
}


void MainGUI::on_mainGUIAlgoSelect_currentIndexChanged(int index) {}


void MainGUI::on_actionAbout_triggered()
{
    AboutWindow *about = new AboutWindow(nullptr); // no parent for it to be a real window
    about->setAttribute(Qt::WA_DeleteOnClose);
    about->setWindowModality(Qt::ApplicationModal);
    about->show();
}


void MainGUI::on_actionConfiguration_triggered()
{
    if (m_libRunStatus != runStatus::IDLE) {
        auto reply = QMessageBox::question(this, "Warning", "ScreenLed is running. Opening settings will stop it, is this OK?",
                                           QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            on_startButt_clicked(); // stop
        } else {
            return;
        }
    }

    SettingsWindow *settings = new SettingsWindow(nullptr, // no parent for it to be a real window
                                                  &m_screenCapWorker->getCurrentConfig(),
                                                  [this](ScreenCapConfig conf) {
                                                      m_screenCapWorker->updateCurrentConfig(conf);
                                                  });
    settings->setAttribute(Qt::WA_DeleteOnClose);
    settings->setWindowModality(Qt::ApplicationModal);
    connect(settings, &QObject::destroyed, this, [this]() {
        // refill when setting window is closed
        populateReceiverStatusRows();
    });
    settings->show();
}


void MainGUI::on_startButt_clicked()
{
    switch (m_libRunStatus) {
    case runStatus::IDLE:
        m_screenLibTh->start();
        ui->startButt->setText("Stop");
        m_libRunStatus = runStatus::RUNNING;
        break;
    case runStatus::RUNNING:
        m_screenCapWorker->stop();
        m_screenLibTh->quit();
        m_screenLibTh->wait();
        ui->startButt->setText("Start");
        m_libRunStatus = runStatus::IDLE;
        break;
    default:
        break;
    }
}


void MainGUI::on_startReceiversButt_clicked()
{
    bool res = false;
    if (!m_receiversRunning ) {
        m_receiversRunning = true;
        res = startReceivers();
        if (res) {
            ui->startReceiversButt->setText("Stop (SSH) Receivers");
        } else {
            m_receiversRunning = false;
        }
    } else {
        m_receiversRunning = false;
        res = stopReceivers();
        if (res) {
            ui->startReceiversButt->setText("Start (SSH) Receivers");
        }
    }
}

bool MainGUI::startReceivers() {
    // App config needs to have network interface selected for this to work so verify it first
    if (m_screenCapWorker->getCurrentConfig().c_preferredLocalNetworkInterface.empty())
    {
        (new ErrorDialog())->Error("Network interface for feedback channel must be set in settings before SSH runner can be used.");
        return false;
    }

    m_rcvRunner = new ReceiverRunnerSSH(m_screenCapWorker->getCurrentConfig().c_clientInfos, QString::fromStdString(m_screenCapWorker->getCurrentConfig().c_preferredLocalNetworkInterface));
    m_rcvRunnerThread = new QThread();
    m_rcvRunner->moveToThread(m_rcvRunnerThread);

    connect(m_rcvRunner, &ReceiverRunnerSSH::outputReady, this, [&](const QString &line) {
        static QRegularExpression regex = QRegularExpression("[\\r\\n]");
        QString trimmed = line;
        trimmed.remove(regex);
        m_rcvRunnerOutput.push_back(trimmed);
        if (m_rcvRunnerOutput.size() > m_maxRcvRunnerLines) {
            m_rcvRunnerOutput.erase(m_rcvRunnerOutput.begin());
        }
        if (m_receiverConsoleOpen && m_receiverConsole != nullptr && !m_receiverConsoleInitialFillOngoing) {
            m_receiverConsole->appendOutput(trimmed);
        }
        qDebug() << "RCVRunner output: " << trimmed;
    });

    connect(m_rcvRunnerThread, &QThread::started, m_rcvRunner, &ReceiverRunnerSSH::start);
    connect(m_rcvRunner, &ReceiverRunnerSSH::finished, m_rcvRunnerThread, &QThread::quit);
    connect(m_rcvRunnerThread, &QThread::finished, m_rcvRunnerThread, &QObject::deleteLater);
    connect(m_rcvRunnerThread, &QThread::finished, this, [this]() {
        // Upon stop, we disable the start button while waiting for the python thread to exit. Once the thred emits finished it should be safe to enable again
        m_rcvRunner = nullptr;
        m_rcvRunnerThread = nullptr;
        ui->startReceiversButt->setEnabled(true);

        if (m_receiversRunning) {
            // if m_receiversRunning is set to True when we hit this, then the rcvRunner has exited unexpectedlys since
            // when stop button is pressed, m_receiversRunning is set to false before doing any real stop activities
            qDebug() << "Seems m_rcvRunnerThread finished unexpectedly";
            m_receiversRunning = false;
            ui->startReceiversButt->setText("Start (SSH) Receivers");
        }
    });

    updateAllSSHReceiverStatusRows("Starting");
    m_rcvRunnerThread->start();
    return true;
}

bool MainGUI::stopReceivers() {
    if (m_rcvRunner != nullptr) {
        QMetaObject::invokeMethod(m_rcvRunner, "stop", Qt::DirectConnection);
        ui->startReceiversButt->setEnabled(false);
    }
    updateAllSSHReceiverStatusRows("Not running");
    return true;
}

void MainGUI::on_actionReceiver_console_triggered()
{
    if (m_receiverConsoleOpen) return;

    m_receiverConsole = new ReceiverConsole();
    m_receiverConsole->setAttribute(Qt::WA_DeleteOnClose);
    connect(m_receiverConsole, &QObject::destroyed, this, [this]() {
        m_receiverConsoleOpen = false;
        m_receiverConsole = nullptr;
    });
    if (!m_rcvRunnerOutput.empty()) { // if there already is something, fill it to the window
        m_receiverConsoleInitialFillOngoing = true;
        m_receiverConsole->fillInitialOutput(m_rcvRunnerOutput);
        m_receiverConsoleInitialFillOngoing = false;
    }
    m_receiverConsoleOpen = true;
    m_receiverConsole->show();
}

void MainGUI::periodicUIUpdate()
{
    auto currTime = std::chrono::system_clock::now();
    if (m_rcvRunner != nullptr)
    {
        for (auto& connection : m_rcvRunner->getAliveTimestamps())
        {
            if (connection.second == std::chrono::system_clock::time_point{}) continue; // alive ts not set -> dont update

            double secondsAgo =
                std::chrono::duration<double>(currTime - connection.second).count();
            QString status = "Running (last alive ";
            status.append(QString::number(secondsAgo, 'f', 1));
            status.append("s ago)");
            updateReceiverStatusRow(connection.first, status);
        }
    }
    if (m_screenCapWorker != nullptr && m_screenCapWorker->m_isRunning)
    {
        ui->statusbar->showMessage("Running FPS: " + QString::number(m_screenCapWorker->m_fps));
    } else {
        ui->statusbar->showMessage("IDLE");
    }
}
