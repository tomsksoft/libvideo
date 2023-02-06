#include <array>

#include "Logs.h"
#include "WindowOutputManagerSW.h"

namespace tomsksoft::libvideo {

void prepare_data(std::vector<uint8_t> &rgba_data) {
	for (uint64_t i = 0; i < rgba_data.size(); i += 4) {
		std::swap(rgba_data[i + 2], rgba_data[i + 3]);
		std::swap(rgba_data[i + 1], rgba_data[i + 2]);
		std::swap(rgba_data[i + 0], rgba_data[i + 1]);
	}

	std::reverse(rgba_data.begin(), rgba_data.end());
}

void WindowOutputManagerSW::initialize_window(HWND hwnd) {
	RECT rect;
	GetClientRect(hwnd, &rect);
	const int window_width = rect.right - rect.left;
	const int window_height = rect.bottom - rect.top;

	BITMAPINFO BitmapInfo;
	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth = window_width;
	BitmapInfo.bmiHeader.biHeight = window_height;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;

	// TODO: Need to test on Windows
	m_wnd_list.emplace_back(hwnd, window_width, window_height, std::move(BitmapInfo));
}

void WindowOutputManagerSW::draw(WinSWData &&frameData) {
	prepare_data(*frameData.rgba_data);

	for (auto const &wnd : m_wnd_list) {
		m_hdc = GetDC(wnd.hwnd);

		StretchDIBits(m_hdc, wnd.width, 0, -wnd.width, wnd.height, 0, 0, wnd.width, wnd.height, frameData.rgba_data->data(),
					  &wnd.BitmapInfo, DIB_RGB_COLORS, SRCCOPY);

		if (frameData.print_stream_info) {
			std::array<std::string, 4> info = {"resolution: " + std::to_string(wnd.width) + "x" + std::to_string(wnd.height),
											   "bitrate: " + std::to_string(frameData.frame_info->bitrate),
											   "framerate: " + std::to_string(frameData.frame_info->framerate),
											   "pts: " + std::to_string(frameData.frame_info->dec_pts)};

			int y_start = 5;
			const int x_start = 5;
			for (unsigned int i = 0; i < info.size(); i++, y_start += 20) {
				TextOut(m_hdc, x_start, y_start, info[i].c_str(), (int)_tcslen(info[i].c_str()));
			}
		}

		ReleaseDC(wnd.hwnd, m_hdc);
	}
}

} // namespace tomsksoft::libvideo
