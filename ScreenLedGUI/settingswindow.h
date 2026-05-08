#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QWidget>
#include <qboxlayout.h>
#include <qlineedit.h>
#include <vector>

#include "Commons.h"

struct ReceiverConfigRow {
    QHBoxLayout *layout;
    QLineEdit *hostEdit;
    QLineEdit *portEdit;
    QComboBox *typeSelect;
};

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = nullptr, ScreenCapConfig *config = nullptr);
    ~SettingsWindow();

private slots:
    void on_SaveButton_clicked();

    void on_addReceiverButton_clicked();

private:
    Ui::SettingsWindow *ui;
    ScreenCapConfig *m_configptr;
    std::vector<ReceiverConfigRow> m_receiverConfigRows;

    // funcs
    void populateSettingsWindow();
    void populateReceiverRows();
    void newReceiverRow();
    static QHBoxLayout* createRow(QLineEdit *hostFiled, QLineEdit *portField, QComboBox *typeSelect);
    bool validateFields();
};

#endif // SETTINGSWINDOW_H
