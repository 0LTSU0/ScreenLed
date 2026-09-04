#include "settingswindow.h"
#include "ui_settingswindow.h"
#include <qcombobox.h>
#include <qscreen.h>

#include <QDir>
#include <QFileDialog>
#include <QDebug>
#include <QNetworkInterface>

SettingsWindow::SettingsWindow(QWidget *parent, ScreenCapConfig config, std::function<void(ScreenCapConfig)> updateConfigFunc)
    : QWidget(parent)
    , ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);
    setWindowTitle("ScreenLed Configuration");
    m_config = config;
    m_updateConfigFunc = updateConfigFunc;
    populateSettingsWindow();
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::populateSettingsWindow()
{
    ui->AutoRunScriptVal->setText(m_config.c_autorunScriptPath);
    ui->DebugPreviewVal->setCheckState(m_config.c_showDebugPreview ? Qt::Checked : Qt::Unchecked);
    ui->DebugSSVal->setCheckState(m_config.c_keepDebugSSOnClipboard ? Qt::Checked : Qt::Unchecked);
    ui->DebugSSintervalVal->setText(QString::number(m_config.c_debugSSInterval));
    ui->ScreenResX->setValue(m_config.c_screenResX);
    ui->ScreenResY->setValue(m_config.c_screenResY);

    if (isWindows()) {
        // windows does not have debug preview option atm (and well on linux also relies on system libs so TODO add to this project
        ui->DebugPreviewVal->setEnabled(false);
    } else {
        ui->DebugSSVal->setEnabled(false);
        ui->DebugSSintervalVal->setEnabled(false);
    }

    populateReceiverRows();
    populateNWInterfaceSelector(QString::fromStdString(m_config.c_preferredLocalNetworkInterface));
    populateScreenAreaSelector(m_config.c_analyzerScreenArea);
    populateAnalyzerDownscaleFactorSelector(m_config.c_analyzerDownscaleFactor);
}

void SettingsWindow::populateReceiverRows() {
    for (auto client : m_config.c_clientInfos) {
        QLineEdit *hostFiled = new QLineEdit();
        QLineEdit *portField = new QLineEdit();
        QComboBox *typeSelect = new QComboBox();
        QLineEdit *ledStripArg = new QLineEdit();
        ledStripArg->setPlaceholderText("LED Strip Arg");

        for (auto& [name, type] : receiverTypeNameMap) {
            typeSelect->addItem(QString::fromStdString(name), static_cast<int>(type));
        }

        // fill values from config
        int idx = typeSelect->findData(static_cast<int>(client.type));
        if (idx != -1) typeSelect->setCurrentIndex(idx);
        hostFiled->setText(QString::fromStdString(client.host));
        portField->setText(QString::number(client.port));
        if (!client.ledStripArg.empty()) { // only fill if not empty to keep placeholder text visible
            ledStripArg->setText(QString::fromStdString(client.ledStripArg));
        }

        QHBoxLayout *rowLayout = createRow(hostFiled, portField, typeSelect, ledStripArg);

        ui->receiverRowContainer->addLayout(rowLayout);
        m_receiverConfigRows.push_back({rowLayout, hostFiled, portField, typeSelect, ledStripArg});
    }
}

void SettingsWindow::newReceiverRow() {
    QLineEdit *hostFiled = new QLineEdit();
    QLineEdit *portField = new QLineEdit();
    QComboBox *typeSelect = new QComboBox();
    QLineEdit *ledStripArg = new QLineEdit();
    for (auto& [name, type] : receiverTypeNameMap) {
        typeSelect->addItem(QString::fromStdString(name), static_cast<int>(type));
    }
    hostFiled->setPlaceholderText("127.0.0.1");
    portField->setPlaceholderText("6967");
    ledStripArg->setPlaceholderText("LED Strip Arg");

    QHBoxLayout *rowLayout = createRow(hostFiled, portField, typeSelect, ledStripArg);
    ui->receiverRowContainer->addLayout(rowLayout);
    m_receiverConfigRows.push_back({rowLayout, hostFiled, portField, typeSelect, ledStripArg});
}

QHBoxLayout* SettingsWindow::createRow(QLineEdit *hostFiled, QLineEdit *portField, QComboBox *typeSelect, QLineEdit *ledStripArg)
{
    QHBoxLayout *rowLayout = new QHBoxLayout();
    rowLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Preferred));
    rowLayout->addWidget(hostFiled, 2); // second arg is stretch
    rowLayout->addWidget(portField, 2); // second arg is stretch
    rowLayout->addWidget(typeSelect, 2); // second arg is stretch
    rowLayout->addWidget(ledStripArg, 2);
    rowLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Preferred));
    rowLayout->setStretch(0,1); //stretch first spacer
    rowLayout->setStretch(5,1); //stretch last spacer
    return rowLayout;
}

bool SettingsWindow::validateFields() {
    bool convOk;
    ui->DebugSSintervalVal->text().toInt(&convOk);
    if (!convOk)
    {
        ui->DebugSSintervalVal->clear(); // TODO error message
        return false;
    }

    for (auto& receiverRow : m_receiverConfigRows)
    {
        if (receiverRow.portEdit->text().isEmpty() || receiverRow.hostEdit->text().isEmpty()) return false;
        receiverRow.portEdit->text().toInt(&convOk);
        if (!convOk) return false; // TODO  error message
    }

    return true;
}

void SettingsWindow::saveConfig() {
    // Note to self: this expects that validateFields() has been run before calling this
    ScreenCapConfig newConf;
    newConf.c_debugSSInterval = ui->DebugSSintervalVal->text().toInt();
    newConf.c_keepDebugSSOnClipboard = ui->DebugSSVal->checkState() == Qt::Checked;
    newConf.c_showDebugPreview = ui->DebugPreviewVal->checkState() == Qt::Checked;
    newConf.c_screenResX = ui->ScreenResX->value();
    newConf.c_screenResY = ui->ScreenResY->value();
    newConf.c_algo = m_config.c_algo; // this is not in settings panel, need to take from currently active config
    newConf.c_autorunScriptPath = ui->AutoRunScriptVal->text();
    newConf.c_preferredLocalNetworkInterface = ui->nwInterfaceVal->currentData().toString().toStdString();
    newConf.c_analyzerScreenArea = static_cast<activeScreenArea>(ui->analAreaVal->currentData().toInt());
    newConf.c_analyzerDownscaleFactor = ui->AnalyzerDownscaleFactorVal->currentData().toInt();

    if (!m_receiverConfigRows.empty()) {
        newConf.c_clientInfos.clear(); // if we have some receivers, default localhost receiver can be removed. Otherwise lets keep it
    }
    for (auto& receiverRow : m_receiverConfigRows) {
        clientInfo newClient;
        newClient.host = receiverRow.hostEdit->text().toStdString();
        newClient.port = receiverRow.portEdit->text().toInt();
        newClient.type = static_cast<receiverType>(receiverRow.typeSelect->currentData().toInt());
        newClient.ledStripArg = receiverRow.ledStripArg->text().toStdString();
        newConf.c_clientInfos.push_back(newClient);
    }

    m_updateConfigFunc(newConf); // will be saved on disk
}

void SettingsWindow::on_SaveButton_clicked()
{
    if (!validateFields()) {
        qDebug() << "validate fields failed on settings window close";
        close(); // TODO error message
        return;
    }

    saveConfig();
    close();
}


void SettingsWindow::on_addReceiverButton_clicked()
{
    newReceiverRow();
}


void SettingsWindow::on_detectResolution_clicked()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect geometry = screen->geometry();
    ui->ScreenResX->setValue(geometry.width());
    ui->ScreenResY->setValue(geometry.height());
}


void SettingsWindow::on_AutoRunScriptSelect_clicked()
{
    QString startDir = QDir::homePath();
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Select autorun python script"),
        startDir,
        tr("Python files (*.py)"));

    if (!fileName.isEmpty())
    {
        qDebug() << "autorun script selected from filedialog: " << fileName;
        ui->AutoRunScriptVal->setText(fileName);
    }
}

void SettingsWindow::populateNWInterfaceSelector(QString selectedName)
{
    qDebug() << "populateNWInterfaceSelector target selected interface" << selectedName;

    ui->nwInterfaceVal->addItem("None", "");
    if (selectedName.isEmpty())
    {
        ui->nwInterfaceVal->setCurrentIndex(0);
    }
    const auto interfaces = QNetworkInterface::allInterfaces();

    std::vector<QNetworkInterface> validInterfaces;
    std::copy_if(
        interfaces.begin(),
        interfaces.end(),
        std::back_inserter(validInterfaces),
        [](const QNetworkInterface &iface) {
            const auto flags = iface.flags();
            return (flags & QNetworkInterface::IsUp) &&
                   (flags & QNetworkInterface::IsRunning) &&
                   !(flags & QNetworkInterface::IsLoopBack);
        });
    int idx = 1;
    for (const auto &interface : validInterfaces)
    {
        QString t = interface.humanReadableName() + ": ";
        for (const QNetworkAddressEntry &entry : interface.addressEntries())
        {
            const QHostAddress &addr = entry.ip();
            if (addr.protocol() == QAbstractSocket::IPv4Protocol)
            {
                t.append(addr.toString());
                break;
            }
        }
        ui->nwInterfaceVal->addItem(t, interface.name());
        if (interface.name() == selectedName)
        {
            ui->nwInterfaceVal->setCurrentIndex(idx);
        }
        idx++;
    }
}

void SettingsWindow::populateScreenAreaSelector(activeScreenArea selectedVal)
{
    int activeIndex = 0;
    int i = 0;
    for (auto& [type, name] : screenAnalysisAreaMap) {
        ui->analAreaVal->addItem(QString::fromStdString(name), static_cast<int>(type));
        if (type == selectedVal) {
            activeIndex = i;
        }
        i++;
    }
    ui->analAreaVal->setCurrentIndex(activeIndex);
}

void SettingsWindow::populateAnalyzerDownscaleFactorSelector(int selectedVal)
{
    int activeIndex = -1;
    for (int i=1; i <= 3; i++)
    {
        ui->AnalyzerDownscaleFactorVal->addItem(QString::number(i), i);
        if (i == selectedVal)
        {
            activeIndex = i - 1;
        }
    }
    ui->AnalyzerDownscaleFactorVal->setCurrentIndex(activeIndex);
}
