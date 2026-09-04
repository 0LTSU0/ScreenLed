#pragma once

#include <libssh2.h>
#include <Commons.h>
#include <string>
#include <atomic>
#include <string>
#include <vector>
#include <utility>
#include <QObject>
#include <QString>
#include <QThread>
#include <QNetworkInterface>

#include "receiverrunnerssh_statuslistener.h"

#ifdef Q_OS_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef SOCKET SocketType;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define INVALID_SOCKET -1
#define closesocket close
typedef int SocketType;
#endif

struct SSHConnection {
    clientInfo client;
    SocketType sock = -1;
    LIBSSH2_SESSION *session = nullptr;
    LIBSSH2_CHANNEL *channel = nullptr;
    std::chrono::system_clock::time_point lastAliveTS{};
};

class ReceiverRunnerSSH : public QObject
{
    Q_OBJECT

public:
    ReceiverRunnerSSH(std::vector<clientInfo> clients, QString nwInterfaceName, QObject *parent = nullptr);
    ~ReceiverRunnerSSH();

    std::vector<std::pair<QString, std::chrono::system_clock::time_point>> getAliveTimestamps();

public slots:
    void start();
    void stop();

signals:
    void outputReady(const QString &line);
    void finished();

private:
    bool connectHost(SSHConnection &conn);
    void cleanup();
    bool initStatusListener();
    void updateConnectionAliveTs(const QByteArray &msg,
                                 const QHostAddress &addr,
                                 quint16 port);
    QString getLocalIPv4();

    std::vector<clientInfo> m_clients;
    std::atomic<bool> m_stopFlag{false};
    std::vector<SSHConnection> m_connections;

    std::string m_username = "pi"; //TODO these need to be configurable per client
    std::string m_password = "raspberry";

    //status listener
    receiverrunnerssh_statuslistener* m_statusListener = nullptr;
    QThread *m_statusListenerThread = nullptr;
    QString m_statusListenerErr;
    QString m_localNWInterfaceName;
    QString m_localIP = getLocalIPv4();
    QNetworkInterface selectCorrectInterface(const std::vector<QNetworkInterface>&);
};

