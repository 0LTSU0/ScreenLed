#include "qrcodewindow.h"
#include "ui_qrcodewindow.h"
#include "qr_code_gen/cpp/qrcodegen.hpp"

using namespace qrcodegen;

QrCodeWindow::QrCodeWindow(std::string url, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QrCodeWindow)
{
    ui->setupUi(this);
    setWindowTitle("Remote config");

    QString qurl = QString::fromStdString(url);
    ui->QRLink->setText(
        QString("<a href=\"%1\">%1</a>").arg(qurl)
    );
    ui->QRLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->QRLink->setOpenExternalLinks(true);

    QrCode qr = QrCode::encodeText(
        url.c_str(),
        QrCode::Ecc::MEDIUM
    );

    const int border = 4;
    const int size = qr.getSize();
    const int imageSize = size + border * 2;

    QImage image(
        imageSize,
        imageSize,
        QImage::Format_RGB32
    );

    image.fill(Qt::white);

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            if (qr.getModule(x, y)) {
                image.setPixel(
                    x + border,
                    y + border,
                    qRgb(0, 0, 0)
                    );
            }
        }
    }

    QPixmap pixmap = QPixmap::fromImage(image).scaled(
        400,
        400,
        Qt::KeepAspectRatio,
        Qt::FastTransformation
    );

    ui->QRCode->setPixmap(pixmap);
}

QrCodeWindow::~QrCodeWindow()
{
    delete ui;
}
