#include "VLCWidget.h"
#include <QResizeEvent>
#include <QMessageBox>

VLCWidget::VLCWidget(QWidget *parent) : QWidget(parent), vlcInstance(nullptr), mediaPlayer(nullptr)
{
    //const char* vlc_args[] = {
    //    "--no-xlib", // Disable X11 integration (optional)
    //    "--plugin-path=C:/Program Files/VideoLAN/VLC/plugins"
    //};

    // Initialize VLC instance
    vlcInstance = libvlc_new(0,  nullptr);
    if (!vlcInstance) {
        QMessageBox::critical(this, "Error", "Failed to initialize libVLC!");
        return;
    }

    // Create VLC media player
    mediaPlayer = libvlc_media_player_new(vlcInstance);
    if (!mediaPlayer) {
        QMessageBox::critical(this, "Error", "Failed to create VLC media player!");
        return;
    }

    // Set the VLC window handle to this QWidget
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_OpaquePaintEvent);
}

VLCWidget::~VLCWidget()
{
    if (mediaPlayer) {
        libvlc_media_player_stop(mediaPlayer);
        libvlc_media_player_release(mediaPlayer);
    }
    if (vlcInstance) {
        libvlc_release(vlcInstance);
    }
}

void VLCWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (mediaPlayer) {
        // Resize the VLC window to match the QWidget size
        libvlc_media_player_set_hwnd(mediaPlayer, (void*)winId());
    }
}

void VLCWidget::playVideo(const QString &filePath)
{
    libvlc_media_t* media = libvlc_media_new_path(vlcInstance, filePath.toStdString().c_str());
    if (!media) {
        QMessageBox::critical(this, "Error", "Failed to open media!");
        return;
    }

    libvlc_media_player_set_media(mediaPlayer, media);
    libvlc_media_release(media);  // Release after setting media

    libvlc_media_player_play(mediaPlayer);
}
