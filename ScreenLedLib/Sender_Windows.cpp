#include "Sender_Windows.h"
#include <QDebug>
#include <math.h>

void Sender::updateConfig(ScreenCapConfig conf)
{
	std::lock_guard lock(m_confMutex);
	m_conf = conf;
}

void Sender::run()
{
	m_running = true;
    if (!initSender())
    {
        qDebug() << "Could not initialize sender";
        return;
    }
	
	while (m_running)
	{
		auto rgbData = m_rgbDataMailbox->get();
        sendRgbData(rgbData);
	}
}

void Sender::stop()
{
	m_running = false;
}

bool Sender::initSender()
{
    std::lock_guard lock(m_confMutex);

    static bool wsaInitialized = false;
    if (!wsaInitialized) {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            qDebug() << "WSAStartup failed: " << result;
            return false;
        }
        wsaInitialized = true;
    }

    for (const auto& client : m_conf.c_clientInfos)
    {
        qDebug() << "Opening UDP Port (Windows)";

        SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == INVALID_SOCKET) {
            qDebug() << "Socket creation failed reason: " << WSAGetLastError();
            return false;
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(client.port);
        inet_pton(AF_INET, client.host.c_str(), &addr.sin_addr);

        m_clientSocks.push_back({ sock, addr });
    }

    m_socksOpen = !m_clientSocks.empty();
    return m_socksOpen;
}

void Sender::deinitSender()
{
    if (m_socksOpen) {
        for (const auto& c : m_clientSocks) {
            qDebug() << "Closing UDP Port (Windows): " << c.sock;
            closesocket(c.sock);
        }
        WSACleanup();
    }
    m_socksOpen = false;
    m_clientSocks.clear();
}

const std::string Sender::createRGBDataString(std::vector<rgbValue>& data) {
    std::string packet = "";
    for (const auto& val : data) {
        packet.append(std::format("{},{},{};", val.r, val.g, val.b));
    }
    return packet;
}

void Sender::sendRgbData(rgbAnalysisResult& data)
{
    const std::string packet = createRGBDataString(data.rgb_values);
    const char* buf = packet.c_str();
    const int len = static_cast<int>(packet.size());

    for (const auto& c : m_clientSocks) {
        int res = sendto(c.sock, buf, len, 0, reinterpret_cast<const sockaddr*>(&c.addr), sizeof(c.addr));
        if (res == SOCKET_ERROR) {
            qDebug() << "sendto failed: " << WSAGetLastError();
        }
    }
    auto delay = std::chrono::steady_clock::now() - data.source_ss_timestamp;
    std::lock_guard<std::mutex> lock(m_ss_to_send_times_mutex);
    m_ss_to_send_times[m_sent_frames % size] = std::chrono::duration_cast<std::chrono::milliseconds>(delay);
}

std::chrono::milliseconds Sender::getSSToSentDelay()
{
    std::lock_guard<std::mutex> lock(m_ss_to_send_times_mutex);

    std::chrono::milliseconds total{ 0 };
    for (const auto& value : m_ss_to_send_times)
    {
        total += value;
    }
    return total / m_ss_to_send_times.size();
}
