#include "maingui.h"
#include "ui_maingui.h"
#include "aboutwindow.h"
#include "settingswindow.h"

#include <QApplication>
#include <QString>

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
        for (auto& row : m_receiverStatusRows) {
            ui->receiverStatusContainer->removeItem(row);
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

void MainGUI::onExitActions() {
    //TODO: Make sure things are stopped on quit

    // Make sure currently selected algo gets saved since its on the main window rather than configuration
    m_screenCapWorker->updateCurrentConfig(m_screenCapWorker->getCurrentConfig());
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
    SettingsWindow *settings = new SettingsWindow(nullptr, &m_screenCapWorker->getCurrentConfig()); // no parent for it to be a real window
    settings->setAttribute(Qt::WA_DeleteOnClose);
    settings->setWindowModality(Qt::ApplicationModal);
    connect(settings, &QObject::destroyed, this, [this]() {
        // refill when setting window is closed
        populateReceiverStatusRows();
    });
    settings->show();
}

