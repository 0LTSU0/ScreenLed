#ifndef RECEIVERRUNNERSSH_STATUSLISTENER_H
#define RECEIVERRUNNERSSH_STATUSLISTENER_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>

class receiverrunnerssh_statuslistener : public QObject
{
    Q_OBJECT
public:
    explicit receiverrunnerssh_statuslistener(QObject *parent = nullptr);

private:
    quint16 m_port = 6969;
    QUdpSocket *m_socket = nullptr;

public slots:
    void start();
    void readPendingDatagrams();
    void stop();

signals:
    void started();
    void error(const QString &reason);
    void udpMessageReceived(QByteArray data, QHostAddress sender, quint16 port);
};

#endif // RECEIVERRUNNERSSH_STATUSLISTENER_H
