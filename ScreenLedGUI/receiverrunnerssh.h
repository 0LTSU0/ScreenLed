#pragma once

#include <libssh2.h>
#include <Commons.h>
#include <string>
#include <atomic>
#include <mutex>
#include <string>
#include <QObject>
#include <QString>

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
};

class ReceiverRunnerSSH : public QObject
{
    Q_OBJECT

public:
    ReceiverRunnerSSH(std::vector<clientInfo> &clients, QObject *parent = nullptr);
    ~ReceiverRunnerSSH();

public slots:
    void start();
    void stop();

signals:
    void outputReady(const QString &line);
    void finished();

private:
private:
    bool connectHost(SSHConnection &conn);
    void cleanup();

    std::vector<clientInfo> m_clients;
    std::vector<SSHConnection> m_connections;
    std::atomic<bool> m_stopFlag{false};

    std::string m_username = "pi"; //TODO these need to be configurable per client
    std::string m_password = "raspberry";
};

