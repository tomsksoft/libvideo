#include <iostream>

#include "TestWindow.h"

LRESULT CALLBACK WindowProc(HWND h_wnd, UINT u_msg, WPARAM w_param, LPARAM l_param) {
	switch (u_msg) {
	case WM_CLOSE: {
		DestroyWindow(h_wnd);
		return 0;
	}
	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}
	case WM_PAINT: {
		return 0;
	}
	default:
		break;
	}
	return DefWindowProc(h_wnd, u_msg, w_param, l_param);
}

Window::Window(int width, int height, int hz_offset, const std::string &win_name) : m_h_instance(GetModuleHandle(nullptr)) {
	const wchar_t *CLASS_NAME = L"Hugos Window Class";
	WNDCLASS wnd_class = {};
	wnd_class.lpszClassName = reinterpret_cast<const char *>(CLASS_NAME);
	wnd_class.hInstance = m_h_instance;
	wnd_class.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
	wnd_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wnd_class.lpfnWndProc = WindowProc;

	RegisterClass(&wnd_class);

	DWORD style = WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;

	RECT rect;
	rect.left = 250 + hz_offset;
	rect.top = 250;
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;

	AdjustWindowRect(&rect, style, false);

	m_hwnd = CreateWindowEx(0, reinterpret_cast<LPCSTR>(CLASS_NAME), reinterpret_cast<LPCSTR>(win_name.c_str()), style, rect.left,
							rect.top, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, m_h_instance, nullptr);

	ShowWindow(m_hwnd, SW_SHOW);
}

Window::~Window() {
	const wchar_t *CLASS_NAME = L"Hugos Window Class";
	UnregisterClass(reinterpret_cast<LPCSTR>(CLASS_NAME), m_h_instance);
}

void Window::wait_frames() {
	bool running = true;
	while (running) {
		if (!process_messages()) {
			stop_encode_decode();
			std::cout << "Closing Window\n";
			running = false;
		}
	}
}

HWND Window::get_hwnd() { return m_hwnd; }

bool Window::process_messages() {
	MSG msg = {};

	while (PeekMessage(&msg, nullptr, 0u, 0u, PM_REMOVE)) {
		if (msg.message == WM_QUIT) {
			return false;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return true;
}