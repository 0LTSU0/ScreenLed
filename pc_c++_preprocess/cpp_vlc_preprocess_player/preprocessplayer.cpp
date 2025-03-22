#include "preprocessplayer.h"
#include "./ui_preprocessplayer.h"
#include <chrono>

#include <QFileDialog>

using namespace std::chrono;

PreProcessPlayer::PreProcessPlayer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PreProcessPlayer)
{
    ui->setupUi(this);

    //vlcWidget = new VLCWidget(this);
    //setCentralWidget(vlcWidget);  // Set VLCWidget as the central widget
    //vlcWidget->playVideo("D:\\Tiedostot\\fina a way or make on cover.mp4");
    vlcPlayer = new VLCPlayer(this);
}

PreProcessPlayer::~PreProcessPlayer()
{
    m_UIUpdateTimer->stop();
    delete ui;
}

void PreProcessPlayer::adjustSeekBar()
{
    int media_length = vlcPlayer->getCurrentMediaLen();
    qDebug() << "Current media len is " << QString::number(media_length);
    ui->VideoTimeSlider->setMaximum(media_length);
}

void PreProcessPlayer::on_SelectVideoButton_clicked()
{
    qDebug("pressed SelectVideoButton");
    auto filePath = QFileDialog::getOpenFileName(this);
    filePath.replace("/", "\\\\");
    qDebug("Selected video: " + filePath.toLatin1());
    ui->SelectVideoPath->setText(filePath);
}

void PreProcessPlayer::updateSeekBar()
{
    qDebug("updateSeekBar() called");
    int curTime = vlcPlayer->getCurrentMediaTime();
    if (!m_timeSliderIsDragging) {
        if (curTime > 0 && curTime < ui->VideoTimeSlider->maximum())
        {
            ui->VideoTimeSlider->setValue(curTime);
        } else if (curTime > 0 && curTime >= ui->VideoTimeSlider->maximum()) {
            ui->VideoTimeSlider->setValue(ui->VideoTimeSlider->maximum());
        } else {
            ui->VideoTimeSlider->setValue(0);
        }
    }

    milliseconds ms(curTime);
    auto h = duration_cast<hours>(ms).count();
    auto m = duration_cast<minutes>(ms).count() % 60;
    auto s = duration_cast<seconds>(ms).count() % 60;

    if (curTime < 1) {
        QString timeLabelStr("00:00");
        ui->VideoTime->setText(timeLabelStr);
    } else if (h > 0) {
        QString timeLabelStr = QString("%1:%2:%3").arg(h, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
        ui->VideoTime->setText(timeLabelStr);
    } else {
        QString timeLabelStr = QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
        ui->VideoTime->setText(timeLabelStr);
    }
}

void PreProcessPlayer::on_playPauseButton_clicked()
{
    bool swapButText = false;
    if (!vlcPlayer->m_isVideoPlaybackStarted) // no video is currently being played by vlc -> start
    {
        bool res = vlcPlayer->playVideo(ui->SelectVideoPath->text());
        if (res)
        {
            adjustSeekBar();
            connect(m_UIUpdateTimer, &QTimer::timeout, this, &PreProcessPlayer::updateSeekBar);
            m_UIUpdateTimer->start(1000);
            swapButText = true;
        }
    } else { // video is loaded to play on vlc -> pause
        vlcPlayer->togglePause();
        swapButText = true;
    }

    if (swapButText)
    {
        if (ui->playPauseButton->text() == "Play")
        {
            ui->playPauseButton->setText("Pause");
        } else {
            ui->playPauseButton->setText("Play");
        }
    }
}


void PreProcessPlayer::on_VideoTimeSlider_sliderPressed()
{
    qDebug("on_VideoTimeSlider_sliderPressed(), setting m_timeSliderIsDragging = true");
    m_timeSliderIsDragging = true;
}


void PreProcessPlayer::on_VideoTimeSlider_sliderReleased()
{
    qDebug("on_VideoTimeSlider_sliderReleased(), setting m_timeSliderIsDragging = false");
    m_timeSliderIsDragging = false;
    vlcPlayer->setMediaTime(ui->VideoTimeSlider->value());
}

