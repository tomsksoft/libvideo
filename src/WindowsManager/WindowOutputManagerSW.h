#ifndef LIBVIDEO_WIN_OUT_MANAGER_SW_H
#define LIBVIDEO_WIN_OUT_MANAGER_SW_H

#include <Windows.h>

#include <vector>

namespace tomsksoft::libvideo {

struct WinSWData {
	std::vector<uint8_t> *rgba_data;
	struct DecodedFrameInfo *frame_info;
	bool print_stream_info;
};

struct SWWindowDescription {
	HWND hwnd;
	int width;
	int height;
	BITMAPINFO BitmapInfo;

	SWWindowDescription(HWND hwnd, int width, int height, BITMAPINFO &&BitmapInfo)
		: hwnd(hwnd), width(width), height(height), BitmapInfo(std::move(BitmapInfo)) {}
};

class WindowOutputManagerSW {
  public:
	WindowOutputManagerSW() = default;
	~WindowOutputManagerSW() = default;

	void initialize_window(HWND hwnd);
	void draw(WinSWData &&frameData);

  private:
	std::vector<SWWindowDescription> m_wnd_list;

	HDC m_hdc = nullptr;
};

} // namespace tomsksoft::libvideo

#endif // LIBVIDEO_WIN_OUT_MANAGER_SW_H