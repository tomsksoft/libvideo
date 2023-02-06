extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

#include "Logs.h"

namespace tomsksoft::libvideo {

#ifdef __ANDROID_API__
#if __ANDROID_API__ > 20
void android_log_callback(void *ptr, int level, const char *fmt, va_list vl) {
	if (level > av_log_get_level())
		return;

	va_list vl2;
	char line[1024];
	static int print_prefix = 1;

	va_copy(vl2, vl);
	av_log_format_line(ptr, level, fmt, vl2, line, sizeof(line), &print_prefix);
	va_end(vl2);

	print_log(ANDROID_LOG_INFO, "FFMPEG", "%s", line);
}
#endif
#endif

LibvideoLog::~LibvideoLog() {
#ifdef __ANDROID_API__
#if __ANDROID_API__ > 20
	print_log(ANDROID_LOG_INFO, "libvideo", "%s", this->m_log_msg.str().c_str());
#endif
#endif

#ifdef _WIN32
#ifdef _MSC_VER
	OutputDebugString(_T(this->m_log_msg.str().c_str()));
#endif
	std::cout << this->m_log_msg.str();
#endif
}

} // namespace tomsksoft::libvideo
