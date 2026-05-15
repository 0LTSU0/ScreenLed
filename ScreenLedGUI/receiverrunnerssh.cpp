#include "receiverrunnerssh.h"
#include <QThread>

ReceiverRunnerSSH::ReceiverRunnerSSH(std::vector<clientInfo> &hosts, QObject *parent)
    : QObject(parent), m_clients(hosts)
{
#ifdef Q_OS_WIN
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    libssh2_init(0);
}

ReceiverRunnerSSH::~ReceiverRunnerSSH()
{
    cleanup();
    libssh2_exit();
#ifdef Q_OS_WIN
    WSACleanup();
#endif
}


bool ReceiverRunnerSSH::connectHost(SSHConnection &conn)
{
    // Open TCP socket
    conn.sock = socket(AF_INET, SOCK_STREAM, 0);
    if (conn.sock == INVALID_SOCKET) {
        emit outputReady(QString("[%1] Failed to create socket").arg(QString::fromStdString(conn.client.host)));
        return false;
    }
    emit outputReady(QString("[%1] Socket created successfully").arg(QString::fromStdString(conn.client.host)));

    sockaddr_in sin{};
    sin.sin_family = AF_INET;
    sin.sin_port = htons(22);
    inet_pton(AF_INET, conn.client.host.c_str(), &sin.sin_addr);

    if (::connect(conn.sock, (sockaddr*)&sin, sizeof(sin)) != 0) {
        emit outputReady(QString("[%1] TCP connect failed").arg(QString::fromStdString(conn.client.host)));
        return false;
    }
    emit outputReady(QString("[%1] TCP connect success").arg(QString::fromStdString(conn.client.host)));

    // Init SSH session
    conn.session = libssh2_session_init();
    if (!conn.session) {
        emit outputReady(QString("[%1] Failed to init SSH session").arg(QString::fromStdString(conn.client.host)));
        return false;
    }
    libssh2_session_set_blocking(conn.session, 1);
    if (libssh2_session_handshake(conn.session, conn.sock) != 0) {
        emit outputReady(QString("[%1] SSH handshake failed").arg(QString::fromStdString(conn.client.host)));
        return false;
    }
    emit outputReady(QString("[%1] SSH Session initialized successfully").arg(QString::fromStdString(conn.client.host)));

    // Authenticate
    if (libssh2_userauth_password(conn.session, m_username.c_str(), m_password.c_str()) != 0) {
        emit outputReady(QString("[%1] SSH auth failed").arg(QString::fromStdString(conn.client.host)));
        return false;
    }
    emit outputReady(QString("[%1] Auth successfull").arg(QString::fromStdString(conn.client.host)));

    // Open interactive shell channel (needed so we can send Ctrl+C later)
    conn.channel = libssh2_channel_open_session(conn.session);
    if (!conn.channel) {
        emit outputReady(QString("[%1] Failed to open SSH channel").arg(QString::fromStdString(conn.client.host)));
        return false;
    }

    // Request PTY so the remote process gets signals properly
    if (libssh2_channel_request_pty(conn.channel, "xterm") != 0) {
        emit outputReady(QString("[%1] Failed to request PTY").arg(QString::fromStdString(conn.client.host)));
        return false;
    }

    // Start shell
    if (libssh2_channel_shell(conn.channel) != 0) {
        emit outputReady(QString("[%1] Failed to start shell").arg(QString::fromStdString(conn.client.host)));
        return false;
    }

    // Set non-blocking for reading
    libssh2_session_set_blocking(conn.session, 0);

    // Send the command
    //std::string cmd = "sudo ./led_server port=" + std::to_string(conn.client.port)
    //                  + " strips=" + conn.client.ledStripArg + "\n";
    std::string cmd = "ls -la \n";
    libssh2_channel_write(conn.channel, cmd.c_str(), cmd.size());

    emit outputReady(QString("[%1] Started led_server").arg(QString::fromStdString(conn.client.host)));
    return true;
}


void ReceiverRunnerSSH::start()
{
    m_stopFlag = false;
    m_connections.clear();

    // Connect to all hosts
    for (const auto &client : m_clients) {
        if (client.type != receiverType::RASPI_SSH) {
            continue;
        }
        SSHConnection conn;
        conn.client = client;
        emit outputReady(QString("[%1] Connecting...").arg(QString::fromStdString(conn.client.host)));
        if (connectHost(conn)) {
            m_connections.push_back(std::move(conn));
        } else {
            emit outputReady(QString("[%1] Failed to connect, skipping").arg(QString::fromStdString(conn.client.host)));
            if (conn.channel) libssh2_channel_free(conn.channel);
            if (conn.session) { libssh2_session_disconnect(conn.session, "error"); libssh2_session_free(conn.session); }
            if (conn.sock != (SocketType)-1) closesocket(conn.sock);
        }
    }

    if (m_connections.empty()) {
        emit outputReady("No connections established, stopping.");
        emit finished();
        return;
    }

    // Poll loop
    char buf[4096];
    while (!m_stopFlag) {
        for (auto &conn : m_connections) {
            if (!conn.channel) continue;

            int rc = libssh2_channel_read(conn.channel, buf, sizeof(buf) - 1);
            if (rc > 0) {
                buf[rc] = '\0';
                QString output = QString("[%1] %2").arg(QString::fromStdString(conn.client.host), QString::fromLocal8Bit(buf, rc));
                emit outputReady(output);
            }

            if (libssh2_channel_eof(conn.channel)) {
                emit outputReady(QString("[%1] Channel closed unexpectedly!").arg(QString::fromStdString(conn.client.host)));
                conn.channel = nullptr;
            }
        }
        QThread::msleep(100);
    }

    cleanup();
    emit finished();
}


void ReceiverRunnerSSH::stop()
{
    // Send Ctrl+C to all channels before setting stop flag
    for (auto &conn : m_connections) {
        if (conn.channel) {
            libssh2_session_set_blocking(conn.session, 1);
            libssh2_channel_write(conn.channel, "\x03", 1);
            libssh2_session_set_blocking(conn.session, 0);
            emit outputReady(QString("[%1] Sent Ctrl+C").arg(QString::fromStdString(conn.client.host)));
        }
    }
    m_stopFlag = true;
}


void ReceiverRunnerSSH::cleanup()
{
    for (auto &conn : m_connections) {
        if (conn.channel) {
            libssh2_channel_send_eof(conn.channel);
            libssh2_channel_free(conn.channel);
            conn.channel = nullptr;
        }
        if (conn.session) {
            libssh2_session_disconnect(conn.session, "Shutting down");
            libssh2_session_free(conn.session);
            conn.session = nullptr;
        }
        if (conn.sock != (SocketType)-1) {
            closesocket(conn.sock);
            conn.sock = -1;
        }
    }
    m_connections.clear();
}
