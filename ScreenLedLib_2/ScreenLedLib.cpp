#include "ScreenLedLib.h"

void ScreenLedLib::updateConfig(ScreenCapConfig conf)
{
	//update our config..
	std::lock_guard lock(m_confMutex);
	m_conf = conf;
	
	//.. and push to "child" threads
	m_screenCapWorker.updateConfig(conf);
	m_AnalyzerWorker.updateConfig(conf);
	m_Sender.updateConfig(conf);
}

void ScreenLedLib::start()
{
	m_screenCapWorker.start();
	m_AnalyzerWorker.start();
	m_Sender.start();
}

void ScreenLedLib::stop()
{
	// shutdown order needs to be reverse so that blocking mailbox get()s dont hang
	m_Sender.stop();
	m_AnalyzerWorker.stop();
	m_screenCapWorker.stop();	
}

double ScreenLedLib::getScreenCapFPS()
{
	return m_screenCapWorker.m_fps;
}

double ScreenLedLib::getAnalyzerAnalyzingTime()
{
	return m_AnalyzerWorker.m_time_per_analyze;
}

std::chrono::milliseconds ScreenLedLib::getAvgSSToSentDelay()
{
	return m_Sender.getSSToSentDelay();
}