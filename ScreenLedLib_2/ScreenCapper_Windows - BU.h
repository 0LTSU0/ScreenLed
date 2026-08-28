#pragma once

#include <QThread>
#include <Windows.h>
#include <atomic>
#include <mutex>
#include <Commons.h>
#include <Mailbox.h>

class ScreenCaptureWorker : public QThread
{
	Q_OBJECT
public:
	ScreenCaptureWorker(ScreenCapConfig conf, LatestOnlyMailbox<RawPixelBuffer>* mailbox)
		: m_conf(conf)
		, m_mailbox(mailbox) { };

	void updateConfig(ScreenCapConfig conf);
	void stop();

	std::atomic<double> m_fps = 0.0;

protected:
	void run() override;

private:
	// funcs
	void screenshot();
	void initScreenShotting();
	void deinitScreenShotting();
	void convertToCommonSSFormat();
	
	// vars
	ScreenCapConfig m_conf;
	mutable std::mutex m_confMutex;

	std::atomic<bool> m_running{ false };
	LatestOnlyMailbox<RawPixelBuffer>* m_mailbox;

	int m_ss_counter = 0;
	DWORD* m_pixelDataPtr = nullptr;
	HDC m_screenDC = nullptr;
	HDC m_memoryDC = nullptr;
	HBITMAP m_bitmap = nullptr;
};