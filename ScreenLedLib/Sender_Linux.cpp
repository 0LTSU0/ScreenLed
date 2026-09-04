#include <Sender_Linux.h>
#include <QDebug>

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
    for (const auto& client : m_conf.c_clientInfos) 
    {
        qDebug() << "Opening UDP Port (Linux)";
        // Create UDP socket 
        int sock = ::socket(AF_INET, SOCK_DGRAM, 0); 
        if (sock < 0) 
        { 
            qDebug() << "Socket creation failed:" << strerror(errno); 
            return false; 
        }
        sockaddr_in addr{}; 
        addr.sin_family = AF_INET; 
        addr.sin_port = htons(client.port);

        // Convert IP address from string to binary form 
        if (inet_pton(AF_INET, client.host.c_str(), &addr.sin_addr) != 1) 
        { 
            qDebug() << "Invalid IP address:" << QString::fromStdString(client.host); 
            ::close(sock); 
            return false; 
        }
        m_clientSocks.push_back({ sock, addr });

        m_socksOpen = !m_clientSocks.empty(); 
        return m_socksOpen;
    }
}

void Sender::deinitSender() 
{ 
    if (m_socksOpen) 
    { 
        for (const auto& c : m_clientSocks) 
        { 
            qDebug() << "Closing UDP Port (Linux): " << c.sock; 
            ::close(c.sock); 
        } 
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
        const ssize_t res = sendto(
            c.sock,
            buf,
            packet.size(),
            0,
            reinterpret_cast<const sockaddr*>(&c.addr),
            sizeof(c.addr)
        );

        if (res == -1) {
            qDebug() << "sendto failed:" << strerror(errno);
        }
    }

    auto delay = std::chrono::steady_clock::now() - data.source_ss_timestamp;

    std::lock_guard<std::mutex> lock(m_ss_to_send_times_mutex);

    m_ss_to_send_times[m_sent_frames % size] =
        std::chrono::duration_cast<std::chrono::milliseconds>(delay);
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
