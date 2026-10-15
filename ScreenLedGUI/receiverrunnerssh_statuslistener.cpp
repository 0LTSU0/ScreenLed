#include "receiverrunnerssh_statuslistener.h"
#include "Log.h"

receiverrunnerssh_statuslistener::receiverrunnerssh_statuslistener(QObject *parent)
    : QObject{parent}
{}

void receiverrunnerssh_statuslistener::start()
{
    m_socket = new QUdpSocket(this);

    if (!m_socket->bind(QHostAddress::AnyIPv4, m_port,
                        QUdpSocket::ShareAddress |
                        QUdpSocket::ReuseAddressHint))
    {
        emit error(m_socket->errorString());
        return;
    }

    connect(m_socket, &QUdpSocket::readyRead,
            this, &receiverrunnerssh_statuslistener::readPendingDatagrams);

    GUI_INFO() << "receiverrunnerssh_statuslistener started";
    emit started();
}

void receiverrunnerssh_statuslistener::stop()
{
    GUI_INFO() << "receiverrunnerssh_statuslistener shutting down";
    if (m_socket) {
        m_socket->close();
    }
}

void receiverrunnerssh_statuslistener::readPendingDatagrams()
{
    while (m_socket->hasPendingDatagrams()) {
        QByteArray data;
        data.resize(m_socket->pendingDatagramSize());

        QHostAddress sender;
        quint16 senderPort;

        m_socket->readDatagram(data.data(), data.size(),
                             &sender, &senderPort);

        emit udpMessageReceived(data, sender, senderPort);
    }
}
