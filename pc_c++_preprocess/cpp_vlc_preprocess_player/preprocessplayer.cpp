#include "preprocessplayer.h"
#include "./ui_preprocessplayer.h"
#include <chrono>

#include <QFileDialog>
#include <QMessageBox>

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

void PreProcessPlayer::on_SelectLedDataButton_pressed()
{
    qDebug("pressed SelectLedDataButton");
    auto filePath = QFileDialog::getOpenFileName(this, "Select a File", QString(), "PPLD Files (*.ppld)");
    filePath.replace("/", "\\\\");
    qDebug("Selected PPLD: " + filePath.toLatin1());
    ui->SelectLedDataPath->setText(filePath);
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
        bool resVLC = vlcPlayer->playVideo(ui->SelectVideoPath->text());
        bool resPPLDLoad = m_udp_worker->loadLedData(ui->SelectLedDataPath->text());
        if (resVLC && resPPLDLoad) // video playback started successfully AND led data load was successfull
        {
            adjustSeekBar();
            connect(m_UIUpdateTimer, &QTimer::timeout, this, &PreProcessPlayer::updateSeekBar);
            m_UIUpdateTimer->start(1000);

            //temp test
            //connect(m_testTimerTimer, &QTimer::timeout, this, &PreProcessPlayer::printCurrentVideoTS);
            //m_testTimerTimer->start(static_cast<int>((1 / 60) * 1000));

            //Move the UDP worker to new thread
            m_udp_worker->moveToThread(m_udp_thread);
            connect(m_udp_thread, &QThread::started, m_udp_worker, &UDPWorker::udpLoop);
            connect(m_udp_worker, &UDPWorker::destroyed, m_udp_thread, &QThread::quit);
            connect(m_udp_thread, &QThread::finished, m_udp_thread, &QObject::deleteLater);
            m_udp_thread->start();

            ui->stopVideoButton->setEnabled(true);
            swapButText = true;
        } else if (resVLC && !resPPLDLoad) { // video playback started successfully but led data was not loaded successfully
            QMessageBox::critical(this, "Error", "Failed to load LED data! Click OK to stop media playback.");
            vlcPlayer->stopVideo();
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

void PreProcessPlayer::printCurrentVideoTS()
{
    qDebug() << "Current media time" << vlcPlayer->getCurrentMediaTime();
}

void PreProcessPlayer::on_stopVideoButton_clicked()
{
    qDebug() << "stopVideoButton Clicked";
    vlcPlayer->stopVideo();
    ui->stopVideoButton->setEnabled(false);
    ui->playPauseButton->setText("Play");
    m_UIUpdateTimer->stop();
    m_udp_thread->requestInterruption();
    m_udp_thread->wait();
    qDebug() << "UDP Thread has exited reiniting m_udp_worker and m_udp_thread";
    m_udp_worker = new UDPWorker();
    m_udp_thread = new QThread();
}

