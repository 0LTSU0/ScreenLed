#ifndef VLCPLAYER_H
#define VLCPLAYER_H

#include <QWidget> //for showing error message boxes directly from VLCPlayer class
#include <vlc/vlc.h>

// Class for handling VLC interaction. Wouldn't really need to be a QWidget but by inheriting it and
// setting the main "preprocessplayer" window as parent we can easily show error messages etc.
// directly from here
class VLCPlayer : public QWidget
{
    Q_OBJECT

public:
    VLCPlayer(QWidget *parent = nullptr);
    ~VLCPlayer();

    bool playVideo(const QString &filepath);
    void stopVideo();
    void togglePause();
    bool is_init_success() { return m_is_init_success; }
    int getCurrentMediaLen();
    int getCurrentMediaTime();
    void setMediaTime(int time);

    bool m_isVideoPlaybackStarted = false;

private:
    bool m_is_init_success = false;
    libvlc_instance_t *m_vlcInstance;
    libvlc_media_player_t *m_mediaPlayer;
    libvlc_media_t* m_media;
};

#endif // VLCPLAYER_H
