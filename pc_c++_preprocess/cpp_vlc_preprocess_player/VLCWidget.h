#ifndef VLCWIDGET_H
#define VLCWIDGET_H

#include <QWidget>
#include <vlc/vlc.h>

class VLCWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VLCWidget(QWidget *parent = nullptr);
    ~VLCWidget();

    void playVideo(const QString &filePath);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    libvlc_instance_t *vlcInstance;
    libvlc_media_player_t *mediaPlayer;
};

#endif // VLCWIDGET_H
