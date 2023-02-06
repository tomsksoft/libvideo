#ifndef LIBVIDEO_LOGS_H
#define LIBVIDEO_LOGS_H

#include <iostream>
#include <sstream>
#include <string>

#ifdef __ANDROID_API__
#if __ANDROID_API__ > 20
#include <android/log.h>
#endif
#endif

#ifdef _MSC_VER
#include <tchar.h>
#include <windows.h>
#endif

namespace tomsksoft::libvideo {

#ifdef __ANDROID_API__
#if __ANDROID_API__ > 20
#define print_log(LOG_LEVEL, TAG, ...) __android_log_print(LOG_LEVEL, TAG, __VA_ARGS__)
void android_log_callback(void *ptr, int level, const char *fmt, va_list vl);
#endif
#endif

class LibvideoLog {
  public:
	LibvideoLog() = default;
	~LibvideoLog();

	template <class T> LibvideoLog &operator<<(const T &data) {
		m_log_msg << data;
		return *this;
	}

  private:
	std::stringstream m_log_msg;
};

struct DecodedFrameInfo {
	int width = -1, height = -1, framerate = -1, dec_pts = -1;
	int64_t bitrate = -1;
};

struct EncodedFrameInfo {
	int initial_size = -1, pkt_size = -1, enc_pts = -1;
};

} // namespace tomsksoft::libvideo

#endif // LIBVIDEO_LOGS_H
