#pragma once

#include <QThread>
#include <vector>
#include <Commons.h>
#include <Mailbox.h>
#include <algomean.h>
#include <algomedian.h>

class Analyzer : public QThread
{
	Q_OBJECT
public:
	Analyzer(ScreenCapConfig conf, LatestOnlyMailbox<RawPixelBuffer>* mailbox, LatestOnlyMailbox<rgbAnalysisResult>* rgbResMailbox)
		: m_config(conf)
		, m_mailbox(mailbox)
		, m_rgbDataMailbox(rgbResMailbox) { };

	void updateConfig(ScreenCapConfig conf);
	void stop();

	std::atomic<double> m_time_per_analyze = 0.0;

protected:
	void run() override;

private:
	ScreenCapConfig m_config;
	mutable std::mutex m_confMutex;
	LatestOnlyMailbox<RawPixelBuffer>* m_mailbox;
	LatestOnlyMailbox<rgbAnalysisResult>* m_rgbDataMailbox;
	std::atomic<bool> m_running{ false };

	rgbAnalysisResult m_analysisResult;
	AlgoMean m_algoMean;
	AlgoMedian m_algoMedian;
};
