#include "settingswindow.h"
#include "ui_settingswindow.h"
#include <qcombobox.h>

SettingsWindow::SettingsWindow(QWidget *parent, ScreenCapConfig *config)
    : QWidget(parent)
    , ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);
    m_configptr = config;
    populateSettingsWindow();
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::populateSettingsWindow()
{
    if (m_configptr == nullptr) {
        return; //TODO: error message
    }

    ui->AutoRunScriptVal->setText(m_configptr->c_autorunScriptPath);
    ui->DebugPreviewVal->setCheckState(m_configptr->c_showDebugPreview ? Qt::Checked : Qt::Unchecked);
    ui->DebugSSVal->setCheckState(m_configptr->c_keepDebugSSOnClipboard ? Qt::Checked : Qt::Unchecked);
    ui->DebugSSintervalVal->setText(QString::number(m_configptr->c_debugSSInterval));
    ui->ScreenResX->setValue(m_configptr->c_screenResX);
    ui->ScreenResY->setValue(m_configptr->c_screenResY);

    if (isWindows()) {
        // windows does not have debug preview option atm (and well on linux also relies on system libs so TODO add to this project
        ui->DebugPreviewVal->setEnabled(false);
    } else {
        ui->DebugSSVal->setEnabled(false);
        ui->DebugSSintervalVal->setEnabled(false);
    }

    populateReceiverRows();
}

void SettingsWindow::populateReceiverRows() {
    for (auto client : m_configptr->c_clientInfos) {
        QLineEdit *hostFiled = new QLineEdit();
        QLineEdit *portField = new QLineEdit();
        QComboBox *typeSelect = new QComboBox();

        for (auto& [name, type] : receiverTypeNameMap) {
            typeSelect->addItem(QString::fromStdString(name), static_cast<int>(type));
        }

        // fill values from config
        int idx = typeSelect->findData(static_cast<int>(client.type));
        if (idx != -1) typeSelect->setCurrentIndex(idx);
        hostFiled->setText(QString::fromStdString(client.host));
        portField->setText(QString::number(client.port));

        QHBoxLayout *rowLayout = createRow(hostFiled, portField, typeSelect);

        ui->receiverRowContainer->addLayout(rowLayout);
        m_receiverConfigRows.push_back({rowLayout, hostFiled, portField, typeSelect});
    }
}

void SettingsWindow::newReceiverRow() {
    QLineEdit *hostFiled = new QLineEdit();
    QLineEdit *portField = new QLineEdit();
    QComboBox *typeSelect = new QComboBox();
    for (auto& [name, type] : receiverTypeNameMap) {
        typeSelect->addItem(QString::fromStdString(name), static_cast<int>(type));
    }
    hostFiled->setPlaceholderText("127.0.0.1");
    portField->setPlaceholderText("6967");

    QHBoxLayout *rowLayout = createRow(hostFiled, portField, typeSelect);
    ui->receiverRowContainer->addLayout(rowLayout);
    m_receiverConfigRows.push_back({rowLayout, hostFiled, portField, typeSelect});
}

QHBoxLayout* SettingsWindow::createRow(QLineEdit *hostFiled, QLineEdit *portField, QComboBox *typeSelect)
{
    QHBoxLayout *rowLayout = new QHBoxLayout();
    rowLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Preferred));
    rowLayout->addWidget(hostFiled, 2); // second arg is stretcg
    rowLayout->addWidget(portField, 1); // second arg is stretcg
    rowLayout->addWidget(typeSelect, 1); // second arg is stretcg
    rowLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Preferred));
    rowLayout->setStretch(0,1); //stretch first spacer
    rowLayout->setStretch(4,1); //stretch first spacer
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
        receiverRow.portEdit->text().toInt(&convOk);
        if (!convOk) return false; // TODO  error message
    }

    return true;
}

void SettingsWindow::on_SaveButton_clicked()
{
    if (!validateFields()) {
        qDebug() << "validate fields failed on settings window close";
        close(); // TODO error message
    }

    close();
}


void SettingsWindow::on_addReceiverButton_clicked()
{
    newReceiverRow();
}

