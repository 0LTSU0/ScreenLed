#pragma once

#include <QThread>
#include <atomic>
#include <string>
#include <format>
#include <array>
#include <mutex>
#include <arpa/inet.h>
#include <unistd.h>
#include <Commons.h>
#include <Mailbox.h>


using Times = std::array<std::chrono::milliseconds, 5>;
constexpr auto size = std::tuple_size_v<Times>;

struct sockclient {
    int sock;
    sockaddr_in addr;
};

class Sender : public QThread
{
	Q_OBJECT
public:
	Sender(ScreenCapConfig conf, LatestOnlyMailbox<rgbAnalysisResult>* rgbDataMailbox)
		: m_conf(conf)
		, m_rgbDataMailbox(rgbDataMailbox) {
	};

	~Sender()
	{
		deinitSender();
	}

	void updateConfig(ScreenCapConfig conf);
	void stop();
	std::chrono::milliseconds getSSToSentDelay();

protected:
	void run() override;

private:
	bool initSender();
	void deinitSender();
	void sendRgbData(rgbAnalysisResult& data);
	const std::string createRGBDataString(std::vector<rgbValue>& data);
	
	ScreenCapConfig m_conf;
	mutable std::mutex m_confMutex;
	LatestOnlyMailbox<rgbAnalysisResult>* m_rgbDataMailbox;
	std::atomic<bool> m_running{ false };
	std::vector<sockclient> m_clientSocks;
	bool m_socksOpen = false;
	
	// perf counter stuff
	Times m_ss_to_send_times{};
	std::mutex m_ss_to_send_times_mutex;
	int m_sent_frames = 0;
};