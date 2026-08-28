#pragma once

#include <QThread>
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
	void convertToCommonSSFormat(std::chrono::steady_clock::time_point);
	bool captureFrame(int res_x, int res_y);
	bool createDuplication();
	
	// vars
	ScreenCapConfig m_conf;
	mutable std::mutex m_confMutex;

	std::atomic<bool> m_running{ false };
	LatestOnlyMailbox<RawPixelBuffer>* m_mailbox;

	int m_ss_counter = 0;
    // X11 and QT have some annoying macro conflicts and the only way I could get it to work is to
    // include the X11 headers in the source file -> cannot use correct types here
    void* m_display = nullptr;      // actually Display*
    unsigned long m_rootWindow = 0; // actually Window
    void* m_image = nullptr;        // actually XImage*
    void* m_shmInfoPtr = nullptr;   // actually XShmSegmentInfo*, allocated/freed in .cpp
    int m_shmWidth = 0;
    int m_shmHeight = 0;
    bool m_shmSupported = false;

    int m_primaryDisplayOffsetX = 0;
    int m_primaryDisplayOffsetY = 0;
};