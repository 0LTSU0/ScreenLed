#ifndef QRCODEWINDOW_H
#define QRCODEWINDOW_H

#include <QDialog>
#include <string>

namespace Ui {
class QrCodeWindow;
}

class QrCodeWindow : public QDialog
{
    Q_OBJECT

public:
    explicit QrCodeWindow(std::string url, QWidget *parent = nullptr);
    ~QrCodeWindow();

private:
    Ui::QrCodeWindow *ui;
};

#endif // QRCODEWINDOW_H
