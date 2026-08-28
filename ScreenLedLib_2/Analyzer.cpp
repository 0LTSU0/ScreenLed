#include <Analyzer.h>
#include <QDebug>
#include <chrono>

void Analyzer::updateConfig(ScreenCapConfig conf)
{
	std::lock_guard lock(m_confMutex);
	m_config = conf;
}

void Analyzer::run()
{
	m_running = true;
	int loopctr = 0;
	double processing_time_per_10 = 0;
	while (m_running)
	{
		auto pixelbuffer = m_mailbox->get();
		loopctr++;
		auto start_process_time = std::chrono::steady_clock::now();
		{
			std::lock_guard lock(m_confMutex);
			bool was_processed = true;
			switch (m_config.c_algo) {
			case ScreenLedAlgorithm::MEAN_DEFAULT:
				m_algoMean.analyzeColors(m_analysisResult.rgb_values, m_config, pixelbuffer);
				break;
			case ScreenLedAlgorithm::MEDIAN:
				m_algoMedian.analyzeColors(m_analysisResult.rgb_values, m_config, pixelbuffer);
				break;
			default:
				qDebug() << "Unknown ScreenLedAlgorithm in analyzer. Cannot process frame";
				was_processed = false;
				break;
			}
			if (was_processed)
			{
				m_analysisResult.source_ss_timestamp = pixelbuffer.ss_timestamp;
				m_rgbDataMailbox->put(m_analysisResult);
			}
		}
		auto end_process_time = std::chrono::steady_clock::now();
		processing_time_per_10 = processing_time_per_10 + std::chrono::duration<double>(end_process_time - start_process_time).count();
		if (loopctr % 10 == 0)
		{
			m_time_per_analyze = processing_time_per_10 / 10;
			processing_time_per_10 = 0;
		}
	}
}

void Analyzer::stop()
{
	m_running = false;
	wait();
}