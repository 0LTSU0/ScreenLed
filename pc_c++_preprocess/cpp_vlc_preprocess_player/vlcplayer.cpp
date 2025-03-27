#include "vlcplayer.h"

#include <QMessageBox>
#include <QThread>

VLCPlayer::VLCPlayer(QWidget *parent) :
    QWidget(parent),
    m_vlcInstance(nullptr),
    m_mediaPlayer(nullptr)
{
    // init vlc player
    m_vlcInstance = libvlc_new(0,  nullptr);
    if (!m_vlcInstance)
    {
        QMessageBox::critical(this, "Error", "Failed to init VLC instance");
        return;
    }
    m_mediaPlayer = libvlc_media_player_new(m_vlcInstance);
    if (!m_mediaPlayer) {
        QMessageBox::critical(this, "Error", "Failed to create VLC media player!");
        return;
    }
    m_is_init_success = true;
}

VLCPlayer::~VLCPlayer()
{
    // Shutdown any possibly runnign vlc stuff
    if (m_mediaPlayer) {
        libvlc_media_player_stop(m_mediaPlayer);
        libvlc_media_player_release(m_mediaPlayer);
    }
    if (m_vlcInstance) {
        libvlc_release(m_vlcInstance);
    }
    if (m_media) {
        libvlc_media_release(m_media);
    }
}

bool VLCPlayer::playVideo(const QString &path)
{
    m_isVideoPlaybackStarted = false;
    if (path.isEmpty())
    {
        QMessageBox::critical(this, "Error", "No video file selected");
        return false;
    }

    m_media = libvlc_media_new_path(m_vlcInstance, path.toStdString().c_str());
    if (!m_media) {
        QMessageBox::critical(this, "Error", "Failed to open selected media!");
        return false;
    }

    libvlc_media_player_set_media(m_mediaPlayer, m_media);
    int error = libvlc_media_player_play(m_mediaPlayer);
    if (error) {
        QMessageBox::critical(this, "Error", "libvlc_media_player_play() failed to start media playback.");
        return false;
    }

    const int max_wait_time = 30000;
    int elapsed_wait_time = 0;
    while (elapsed_wait_time < max_wait_time)
    {
        auto vlc_state = libvlc_media_player_get_state(m_mediaPlayer);
        qDebug() << "Waiting for vlc to finish opening file. Current vlc_status: " << QString::number(vlc_state);
        if (vlc_state == libvlc_Playing) {
            m_isVideoPlaybackStarted = true;
            return true;
        }
        else if (vlc_state == libvlc_Error || vlc_state == libvlc_Stopped || vlc_state == libvlc_Ended) {
            m_isVideoPlaybackStarted = false;
            return false;
        }
        QThread::msleep(100);
    }

    return false;
}

void VLCPlayer::stopVideo()
{
    if (libvlc_media_player_get_state(m_mediaPlayer) == libvlc_Playing) {
        qDebug() << "VLC stopping video";
        libvlc_media_player_stop(m_mediaPlayer);
    }
    m_isVideoPlaybackStarted = false;
}

int VLCPlayer::getCurrentMediaLen()
{
    return libvlc_media_get_duration(m_media);
}

void VLCPlayer::togglePause()
{
    libvlc_media_player_pause(m_mediaPlayer);
}

int VLCPlayer::getCurrentMediaTime()
{
    return libvlc_media_player_get_time(m_mediaPlayer);
}

void VLCPlayer::setMediaTime(int time)
{
    if (!m_isVideoPlaybackStarted) {
        return;
    }
    libvlc_media_player_set_time(m_mediaPlayer, time);
}

