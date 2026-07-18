#include "receiverrunnerssh.h"
#include <QThread>
#include <QDebug>
#include <QEventLoop>
#include <QNetworkInterface>

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
    std::string cmd = "sudo ./led_receiver port=" + std::to_string(conn.client.port)
                      + " strips=" + conn.client.ledStripArg
                      + " host=" + m_localIP.toStdString() + "\n";
    //std::string cmd = "./randStr.sh \n";
    libssh2_channel_write(conn.channel, cmd.c_str(), cmd.size());

    emit outputReady(QString("[%1] Started led_server").arg(QString::fromStdString(conn.client.host)));
    return true;
}


bool ReceiverRunnerSSH::initStatusListener()
{
    m_statusListener = new receiverrunnerssh_statuslistener();
    m_statusListenerThread = new QThread(this);
    m_statusListener->moveToThread(m_statusListenerThread);

    connect(m_statusListenerThread, &QThread::started,
            m_statusListener, &receiverrunnerssh_statuslistener::start);


    // TEMP
    connect(m_statusListener,
            &receiverrunnerssh_statuslistener::udpMessageReceived,
            this,
            &ReceiverRunnerSSH::updateConnectionAliveTs,
            Qt::DirectConnection);

    QEventLoop loop;
    bool ok = false;

    connect(m_statusListener, &receiverrunnerssh_statuslistener::started, &loop, [&]{
        ok = true;
        loop.quit();
    });

    connect(m_statusListener, &receiverrunnerssh_statuslistener::error, &loop, [&](const QString &msg){
        //qCritical() << msg;
        m_statusListenerErr = msg;
        ok = false;
        loop.quit();
    });

    m_statusListenerThread->start();
    loop.exec(); // wait for init to fail or succeed

    return ok;
}


void ReceiverRunnerSSH::start()
{
    if (!initStatusListener()) {
        emit outputReady("ERROR: Could not start statusListener!");
        emit outputReady(m_statusListenerErr);
        emit finished();
        return;
    }

    if (m_localIP.isEmpty())
    {
        emit outputReady("ERROR: Local ip is not known. Cannot provide our IP to led receivers");
        emit finished();
        return;
    }

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
        QThread::msleep(50);
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

    if (m_statusListener && m_statusListenerThread) {

        // Run stop() in the listener's own thread
        QMetaObject::invokeMethod(
            m_statusListener,
            "stop",
            Qt::BlockingQueuedConnection
        );

        m_statusListenerThread->quit();
        m_statusListenerThread->wait();

        m_statusListener = nullptr;
        m_statusListenerThread = nullptr;
    }
}

void ReceiverRunnerSSH::updateConnectionAliveTs(const QByteArray &msg,
                                                const QHostAddress &addr,
                                                quint16 port)
{
    qDebug() << msg << addr << port;
    for (auto& connection : m_connections)
    {
        auto client = connection.client;
        if (client.host == addr.toString().toStdString())
        {
            if (msg.toStdString().find("alive") != std::string::npos)
            {
                connection.lastAliveTS = std::chrono::system_clock::now();
            }
        }
    }
}

QString ReceiverRunnerSSH::getLocalIPv4()
{
    // TODO: there should be a way to select correct interface or ip
    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &iface : interfaces)
    {
        // Skip interfaces that are down or loopback
        if (!(iface.flags() & QNetworkInterface::IsUp) ||
            !(iface.flags() & QNetworkInterface::IsRunning) ||
            (iface.flags() & QNetworkInterface::IsLoopBack))
        {
            continue;
        }

        for (const QNetworkAddressEntry &entry : iface.addressEntries())
        {
            const QHostAddress &addr = entry.ip();

            if (addr.protocol() == QAbstractSocket::IPv4Protocol)
            {
                qDebug() << "getLocalIPv4 returning" << addr.toString();
                return addr.toString();
            }
        }
    }

    return {};
}

std::vector<std::pair<QString, std::chrono::system_clock::time_point>> ReceiverRunnerSSH::getAliveTimestamps()
{
    std::vector<std::pair<QString, std::chrono::system_clock::time_point>> res;
    for (auto& conn : m_connections)
    {
        res.emplace_back(QString::fromStdString(conn.client.host), conn.lastAliveTS);
    }
    return res;
}
