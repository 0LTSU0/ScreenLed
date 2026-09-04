#include "ScreenCapper_Windows.h"

#include <QDebug>

void convertRowBGRAtoBGR_SSE(const unsigned char* src, unsigned char* dst, int width) {
	const __m128i mask = _mm_setr_epi8(
		0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, -1, -1, -1, -1);

	int x = 0;
	for (; x + 4 <= width; x += 4) {
		__m128i pix = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src + x * 4));
		__m128i shuffled = _mm_shuffle_epi8(pix, mask);
		std::memcpy(dst + x * 3, &shuffled, 12);
	}
	for (; x < width; ++x) {
		dst[x * 3 + 0] = src[x * 4 + 0];
		dst[x * 3 + 1] = src[x * 4 + 1];
		dst[x * 3 + 2] = src[x * 4 + 2];
	}
}

void ScreenCaptureWorker::updateConfig(ScreenCapConfig conf)
{
	std::lock_guard lock(m_confMutex);
	m_conf = conf;
}

void ScreenCaptureWorker::run()
{
	initScreenShotting();
	m_running = true;
	
	int loopctr = 0;
	auto fps_ctr_start = std::chrono::steady_clock::now();
	while (m_running)
	{
		screenshot();
		loopctr++;
		if (loopctr == 10)
		{
			auto now = std::chrono::steady_clock::now();
			double elapsed = std::chrono::duration<double>(now - fps_ctr_start).count();
			m_fps = 10.0 / elapsed;
			fps_ctr_start = now;
			loopctr = 0;
		}
	}
}

void ScreenCaptureWorker::stop()
{
	m_running = false;
	wait();
	deinitScreenShotting();
}

void ScreenCaptureWorker::initScreenShotting()
{
	int res_x, res_y;
	{
		std::lock_guard lock(m_confMutex);
		res_x = m_conf.c_screenResX;
		res_y = m_conf.c_screenResY;
	}

	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDevice(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
		nullptr, 0, D3D11_SDK_VERSION,
		&m_d3dDevice, &featureLevel, &m_d3dContext);

	if (FAILED(hr)) {
		qDebug() << "D3D11CreateDevice failed:" << hr;
		return;
	}

	if (!createDuplication()) {
		qDebug() << "Initial DXGI duplication setup failed";
		return;
	}

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = res_x;
	desc.Height = res_y;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.BindFlags = 0;

	hr = m_d3dDevice->CreateTexture2D(&desc, nullptr, &m_stagingTex);
	if (FAILED(hr)) {
		qDebug() << "CreateTexture2D (staging) failed:" << hr;
	}

	m_pixelDataPtr = new DWORD[static_cast<size_t>(res_x) * res_y];
	memset(m_pixelDataPtr, 0, static_cast<size_t>(res_x) * res_y * sizeof(DWORD));
}

bool ScreenCaptureWorker::createDuplication()
{
	// Release old duplication if re-creating after loss
	if (m_duplication) {
		m_duplication->Release();
		m_duplication = nullptr;
	}

	IDXGIDevice* dxgiDevice = nullptr;
	HRESULT hr = m_d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
	if (FAILED(hr)) return false;

	IDXGIAdapter* adapter = nullptr;
	hr = dxgiDevice->GetAdapter(&adapter);
	dxgiDevice->Release();
	if (FAILED(hr)) return false;

	IDXGIOutput* output = nullptr;
	hr = adapter->EnumOutputs(0, &output); // monitor index 0 — TODO: make configurable
	adapter->Release();
	if (FAILED(hr)) return false;

	IDXGIOutput1* output1 = nullptr;
	hr = output->QueryInterface(__uuidof(IDXGIOutput1), (void**)&output1);
	output->Release();
	if (FAILED(hr)) return false;

	hr = output1->DuplicateOutput(m_d3dDevice, &m_duplication);
	output1->Release();
	if (FAILED(hr)) {
		qDebug() << "DuplicateOutput failed:" << hr;
		return false;
	}

	return true;
}

void ScreenCaptureWorker::deinitScreenShotting()
{
	if (m_stagingTex) { m_stagingTex->Release();  m_stagingTex = nullptr; }
	if (m_duplication) { m_duplication->Release(); m_duplication = nullptr; }
	if (m_d3dContext) { m_d3dContext->Release();  m_d3dContext = nullptr; }
	if (m_d3dDevice) { m_d3dDevice->Release();   m_d3dDevice = nullptr; }
}

bool ScreenCaptureWorker::captureFrame(int res_x, int res_y)
{
	if (!m_duplication) return false;

	IDXGIResource* desktopResource = nullptr;
	DXGI_OUTDUPL_FRAME_INFO frameInfo;

	HRESULT hr = m_duplication->AcquireNextFrame(500, &frameInfo, &desktopResource);

	if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
		return false;
	}
	if (hr == DXGI_ERROR_ACCESS_LOST) {
		qDebug() << "DXGI access lost, recreating duplication";
		createDuplication();
		return false;
	}
	if (FAILED(hr)) {
		qDebug() << "AcquireNextFrame failed:" << hr;
		return false;
	}

	ID3D11Texture2D* desktopTex = nullptr;
	hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&desktopTex);
	desktopResource->Release();
	if (FAILED(hr)) {
		m_duplication->ReleaseFrame();
		return false;
	}

	m_d3dContext->CopyResource(m_stagingTex, desktopTex);
	desktopTex->Release();

	D3D11_MAPPED_SUBRESOURCE mapped;
	hr = m_d3dContext->Map(m_stagingTex, 0, D3D11_MAP_READ, 0, &mapped);
	if (FAILED(hr)) {
		m_duplication->ReleaseFrame();
		return false;
	}

	BYTE* dstBase = reinterpret_cast<BYTE*>(m_pixelDataPtr);
	const BYTE* srcBase = static_cast<const BYTE*>(mapped.pData);
	const size_t rowBytes = static_cast<size_t>(res_x) * 4;

	// DXGI gives top-down rows; flip into bottom-up order to match
	// the convention the rest of the pipeline (clipboard, analyzer) expects.
	for (int y = 0; y < res_y; ++y) {
		const BYTE* srcRow = srcBase + static_cast<size_t>(y) * mapped.RowPitch;
		BYTE* dstRow = dstBase + static_cast<size_t>(res_y - 1 - y) * rowBytes;
		memcpy(dstRow, srcRow, rowBytes);
	}

	m_d3dContext->Unmap(m_stagingTex, 0);
	m_duplication->ReleaseFrame();
	return true;
}

void ScreenCaptureWorker::screenshot()
{
	int res_x, res_y;
	bool keep_on_clipboard;
	int debugSSInterval;
	{
		std::lock_guard lock(m_confMutex);
		res_x = m_conf.c_screenResX;
		res_y = m_conf.c_screenResY;
		keep_on_clipboard = m_conf.c_keepDebugSSOnClipboard;
		debugSSInterval = m_conf.c_debugSSInterval;
	}

	bool gotFrame = captureFrame(res_x, res_y);
	if (!gotFrame) {
		// No new frame (timeout) or a transient error — skip processing this tick.
		// m_pixelDataPtr still holds the last good frame.
		return;
	}
	auto ss_time = std::chrono::steady_clock::now();

	if (keep_on_clipboard) {
		if (m_ss_counter % debugSSInterval == 0) {

			size_t pixelCount = static_cast<size_t>(res_x) * res_y;

			std::vector<DWORD> clipPixels(m_pixelDataPtr, m_pixelDataPtr + pixelCount);
			for (auto& px : clipPixels) {
				px |= 0xFF000000; // force alpha opaque
			}

			BITMAPINFOHEADER bih = {};
			bih.biSize = sizeof(BITMAPINFOHEADER);
			bih.biWidth = res_x;
			bih.biHeight = res_y;
			bih.biPlanes = 1;
			bih.biBitCount = 32;
			bih.biCompression = BI_RGB;
			bih.biSizeImage = static_cast<DWORD>(pixelCount * 4);

			HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + bih.biSizeImage);
			if (hMem) {
				BYTE* pMem = static_cast<BYTE*>(GlobalLock(hMem));
				if (pMem) {
					memcpy(pMem, &bih, sizeof(BITMAPINFOHEADER));
					memcpy(pMem + sizeof(BITMAPINFOHEADER), clipPixels.data(), bih.biSizeImage);
					GlobalUnlock(hMem);

					if (OpenClipboard(NULL)) {
						EmptyClipboard();
						if (!SetClipboardData(CF_DIB, hMem)) {
							qDebug() << "Failed to set clipboard contents:" << GetLastError();
							GlobalFree(hMem);
						}
						CloseClipboard();
					}
					else {
						GlobalFree(hMem);
					}
				}
				else {
					GlobalFree(hMem);
				}
			}
		}
	}

	m_ss_counter++;
	convertToCommonSSFormat(ss_time);
}

void ScreenCaptureWorker::convertToCommonSSFormat(std::chrono::steady_clock::time_point ss_time)
{
	int srcW;
	int srcH;
	int factor;
	{
		std::lock_guard lock(m_confMutex);
		srcW = m_conf.c_screenResX;
		srcH = m_conf.c_screenResY;
		factor = std::max(1, m_conf.c_analyzerDownscaleFactor);
	}
	const int outW = srcW / factor;
	const int outH = srcH / factor;
	
	if (outW <= 0 || outH <= 0) return;
	RawPixelBuffer CommonPixelData;
	CommonPixelData.rgb.resize(static_cast<size_t>(outW) * outH * 3);
	CommonPixelData.width = outW;
	CommonPixelData.height = outH;
	CommonPixelData.ss_timestamp = ss_time;

	const int srcStride = srcW; // stride is in pixels (DWORDs), not bytes, for this buffer
	const unsigned char* src = reinterpret_cast<const unsigned char*>(m_pixelDataPtr);
	unsigned char* dst = CommonPixelData.rgb.data();

	for (int y = 0; y < outH; ++y) {
		const unsigned char* srcRow = src + static_cast<size_t>(y * factor) * srcStride * 4;
		unsigned char* dstRow = dst + static_cast<size_t>(y) * outW * 3;

		if (factor == 1) {
			// contiguous row -> SIMD shuffle applies correctly
			convertRowBGRAtoBGR_SSE(srcRow, dstRow, outW);
		}
		else {
			// strided sampling -> scalar
			for (int x = 0; x < outW; ++x) {
				const unsigned char* p = srcRow + (static_cast<size_t>(x) * factor) * 4;
				dstRow[x * 3 + 0] = p[0]; // B
				dstRow[x * 3 + 1] = p[1]; // G
				dstRow[x * 3 + 2] = p[2]; // R
			}
		}
	}

	m_mailbox->put(CommonPixelData);
}
