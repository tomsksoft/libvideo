#ifndef WINDOW_H
#define WINDOW_H

#include <Windows.h>
#include <vector>
#include <functional>
#include <variant>

#include <d3d11.h>
#include <dxgi1_2.h>

LRESULT CALLBACK WindowProc(HWND h_wnd, UINT u_msg, WPARAM w_param, LPARAM l_param);

class Window {
  public:
	Window(int width, int height, int hz_offset, const std::string &win_name);
	~Window();

	HWND get_hwnd();
	bool process_messages();
	void wait_frames();
	std::function<void(void)> stop_encode_decode;

  private:
	HINSTANCE m_h_instance;
	HWND m_hwnd;
};

#endif // WINDOW_H
