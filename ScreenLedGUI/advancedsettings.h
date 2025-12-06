#ifndef ADVANCEDSETTINGS_H
#define ADVANCEDSETTINGS_H

#include <QWidget>
#include "Commons.h"

namespace Ui {
class AdvancedSettings;
}

class AdvancedSettings : public QWidget
{
    Q_OBJECT

public:
    explicit AdvancedSettings(const ScreenCapConfig &conf, QWidget *parent = nullptr);
    ~AdvancedSettings();

private slots:
    void on_saveAdvanced_clicked();

private:
    // vars
    QWidget *m_parentWindow = nullptr;

    // functions
    void fillForm(const ScreenCapConfig&);

    Ui::AdvancedSettings *ui;
};

#endif // ADVANCEDSETTINGS_H
