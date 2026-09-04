#pragma once

#include <QThread>
#include <Windows.h>
#include <atomic>
#include <mutex>
#include <chrono>
#include <Commons.h>
#include <Mailbox.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

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
	ID3D11Device* m_d3dDevice = nullptr;
	ID3D11DeviceContext* m_d3dContext = nullptr;
	IDXGIOutputDuplication* m_duplication = nullptr;
	ID3D11Texture2D* m_stagingTex = nullptr;
	DWORD* m_pixelDataPtr = nullptr;
};
