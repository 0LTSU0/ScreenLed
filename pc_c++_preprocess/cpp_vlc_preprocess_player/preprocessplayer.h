#ifndef PREPROCESSPLAYER_H
#define PREPROCESSPLAYER_H

#include <QMainWindow>
//#include "VLCWidget.h"
#include "qthread.h"
#include "vlcplayer.h"
#include "qtimer.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class PreProcessPlayer;
}
QT_END_NAMESPACE

class PreProcessPlayer : public QMainWindow
{
    Q_OBJECT

public:
    PreProcessPlayer(QWidget *parent = nullptr);
    ~PreProcessPlayer();

    bool get_vlc_init_success()
    {
        return vlcPlayer->is_init_success();
    }

private slots:
    void on_SelectVideoButton_clicked();

    void on_playPauseButton_clicked();

    void on_VideoTimeSlider_sliderPressed();

    void on_VideoTimeSlider_sliderReleased();

private:
    bool m_timeSliderIsDragging = false;

    Ui::PreProcessPlayer *ui;
    VLCPlayer *vlcPlayer;

    QTimer *m_UIUpdateTimer = new QTimer(this);

    void updateSeekBar();
    void adjustSeekBar();
};
#endif // PREPROCESSPLAYER_H
