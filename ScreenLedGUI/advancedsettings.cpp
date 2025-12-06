#include "advancedsettings.h"
#include "ui_advancedsettings.h"
#include "screenledgui.h"

AdvancedSettings::AdvancedSettings(const ScreenCapConfig &conf, QWidget *parent)
    : QWidget(nullptr) // to keep this separate window
    , m_parentWindow(parent)
    , ui(new Ui::AdvancedSettings)
{
    ui->setupUi(this);
    fillForm(conf);
}

AdvancedSettings::~AdvancedSettings()
{
    delete ui;
}

void AdvancedSettings::fillForm(const ScreenCapConfig& conf){
    ui->screenResX->setValue(conf.c_screenResX);
    ui->screenResY->setValue(conf.c_screenResY);
    ui->debugSSIntervalVal->setValue(conf.c_debugSSInterval);

#ifdef _WIN32
    ui->debugSSVal->setChecked(conf.c_keepDebugSSOnClipboard);
    ui->debugPreviewVal->setEnabled(false);
#else
    ui->debugPreviewVal->setChecked(conf.c_showDebugPreview);
    ui->debugSSVal->setEnabled(false);
#endif
}

void AdvancedSettings::on_saveAdvanced_clicked()
{
    ScreenLedGUI* mainWindow = qobject_cast<ScreenLedGUI*>(m_parentWindow);
    if (!mainWindow)
    {
        return;
    }

    auto conf = mainWindow->getScreenCapWorkerConf();
    conf.c_screenResX = ui->screenResX->value();
    conf.c_screenResY = ui->screenResY->value();
    conf.c_debugSSInterval = ui->debugSSIntervalVal->value();
    conf.c_keepDebugSSOnClipboard = ui->debugSSVal->isChecked();
    conf.c_showDebugPreview = ui->debugPreviewVal->isChecked();
    mainWindow->setScreenCapWorkerConf(conf); // TODO if false show error
    close();
}

