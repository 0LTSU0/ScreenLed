#pragma once

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "ScreenCapper_Windows.h"
#include "Sender_Windows.h"
#else
#include "ScreenCapper_Linux.h"
#include "Sender_Linux.h"
#endif // _WIN32

#include <QObject>
#include <string>
#include <mutex>
#include <atomic>
#include <vector>
#include <chrono>
#include "Commons.h"
#include "Mailbox.h"
#include "Analyzer.h"

class ScreenLedLib : public QObject
{
	Q_OBJECT
public:
	ScreenLedLib(ScreenCapConfig conf)
		: m_conf(conf)
		, m_screenCapWorker(m_conf, &m_screenshotMailbox)
		, m_AnalyzerWorker(m_conf, &m_screenshotMailbox, &m_rgbDataMailbox)
		, m_Sender(m_conf, &m_rgbDataMailbox) { };

	void updateConfig(ScreenCapConfig conf);

	void start();
	void stop();

	double getScreenCapFPS();
	double getAnalyzerAnalyzingTime();
	std::chrono::milliseconds getAvgSSToSentDelay();

	//vars
	LatestOnlyMailbox<RawPixelBuffer> m_screenshotMailbox;
	LatestOnlyMailbox<rgbAnalysisResult> m_rgbDataMailbox;

private:
	ScreenCapConfig m_conf;
	std::mutex m_confMutex;

	ScreenCaptureWorker m_screenCapWorker;
	Analyzer m_AnalyzerWorker;
	Sender m_Sender;
};