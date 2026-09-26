#ifndef ALGOCONFIGWINDOW_H
#define ALGOCONFIGWINDOW_H

#include <QDialog>

namespace Ui {
class AlgoConfigWindow;
}

class AlgoConfigWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AlgoConfigWindow(QWidget *parent = nullptr, QWidget *algoConfigWidget = nullptr);
    ~AlgoConfigWindow();

private:
    Ui::AlgoConfigWindow *ui;
};

#endif // ALGOCONFIGWINDOW_H
