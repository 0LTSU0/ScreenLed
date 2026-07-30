#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class ErrorDialog;
}

class ErrorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ErrorDialog(QWidget *parent = nullptr);
    ~ErrorDialog();

    void Error(QString);
    void Warning(QString);

private:
    Ui::ErrorDialog *ui;
};

#endif // ERRORDIALOG_H
