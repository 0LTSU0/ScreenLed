#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QWidget>
#include <qboxlayout.h>
#include <qlineedit.h>
#include <vector>
#include <functional>
#include "qcombobox.h"

#include "Commons.h"

struct ReceiverConfigRow {
    QHBoxLayout *layout;
    QLineEdit *hostEdit;
    QLineEdit *portEdit;
    QComboBox *typeSelect;
    QLineEdit *ledStripArg;
};

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = nullptr, ScreenCapConfig config = ScreenCapConfig(), std::function<void(ScreenCapConfig)> = nullptr);
    ~SettingsWindow();

private slots:
    void on_SaveButton_clicked();

    void on_addReceiverButton_clicked();

    void on_detectResolution_clicked();

    void on_AutoRunScriptSelect_clicked();

private:
    Ui::SettingsWindow *ui;
    ScreenCapConfig m_config;
    std::function<void(ScreenCapConfig)> m_updateConfigFunc;
    std::vector<ReceiverConfigRow> m_receiverConfigRows;

    // funcs
    void populateSettingsWindow();
    void populateReceiverRows();
    void populateNWInterfaceSelector(QString selectedName);
    void populateScreenAreaSelector(activeScreenArea selectedVal);
    void populateAnalyzerDownscaleFactorSelector(int selectedVal);
    void newReceiverRow();
    static QHBoxLayout* createRow(QLineEdit *hostFiled, QLineEdit *portField, QComboBox *typeSelect, QLineEdit *ledStripArg);
    bool validateFields();
    void saveConfig();
};

#endif // SETTINGSWINDOW_H
