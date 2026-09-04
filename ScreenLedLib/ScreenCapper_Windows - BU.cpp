#include "ScreenCapper_Windows.h"

#include <QDebug>
#include <chrono>

//TEMP
#include <iostream>

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
	int res_x;
	int res_y;
	{
		std::lock_guard lock(m_confMutex);
		res_x = m_conf.c_screenResX;
		res_y = m_conf.c_screenResY;
	}
	m_screenDC = GetDC(nullptr);
	m_memoryDC = CreateCompatibleDC(m_screenDC);

	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = res_x;
	bmi.bmiHeader.biHeight = res_y;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	void* pBits = nullptr;
	m_bitmap = CreateDIBSection(m_screenDC, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
	m_pixelDataPtr = static_cast<DWORD*>(pBits);
}

void ScreenCaptureWorker::deinitScreenShotting()
{
	DeleteObject(m_bitmap);
	DeleteDC(m_memoryDC);
	ReleaseDC(nullptr, m_screenDC);
}

void ScreenCaptureWorker::screenshot()
{
	int res_x;
	int res_y;
	bool keep_on_clipboard;
	int debugSSInterval;
	{
		std::lock_guard lock(m_confMutex);
		res_x = m_conf.c_screenResX;
		res_y = m_conf.c_screenResY;
		keep_on_clipboard = m_conf.c_keepDebugSSOnClipboard;
		debugSSInterval = m_conf.c_debugSSInterval;
	}

	HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(m_memoryDC, m_bitmap));
	BitBlt(m_memoryDC, 0, 0, res_x, res_y, m_screenDC, 0, 0, SRCCOPY);
	SelectObject(m_memoryDC, hOldBitmap);

	// Pull pixels into m_pixelData
	//BITMAPINFO bmi;
	//memset(&bmi, 0, sizeof(BITMAPINFO));
	//bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	//bmi.bmiHeader.biWidth = res_x;
	//bmi.bmiHeader.biHeight = res_y;
	//bmi.bmiHeader.biPlanes = 1;
	//bmi.bmiHeader.biBitCount = 32;
	//bmi.bmiHeader.biCompression = BI_RGB;
	//GetDIBits(m_memoryDC, m_bitmap, 0, res_y, m_pixelData.get(), &bmi, DIB_RGB_COLORS);

	if (keep_on_clipboard) {
		if (m_ss_counter % debugSSInterval == 0) {

			size_t pixelCount = static_cast<size_t>(res_x) * res_y;

			// Copy + force alpha opaque so nothing treats this as transparent/black
			std::vector<DWORD> clipPixels(m_pixelDataPtr, m_pixelDataPtr + pixelCount);
			for (auto& px : clipPixels) {
				px |= 0xFF000000;
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

	convertToCommonSSFormat();
}

void ScreenCaptureWorker::convertToCommonSSFormat()
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