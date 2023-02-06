extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

#include "YUVtoRGBA.h"

#include "Logs.h"

namespace tomsksoft::libvideo {
YUVtoRGBAConvertor::YUVtoRGBAConvertor(int width, int height) {
	m_rgba_decoded_data.insert(m_rgba_decoded_data.begin(), width * height * 4, 0);
	m_yuv_to_rgba_ctx = sws_getContext(width, height, AV_PIX_FMT_YUV420P, width, height, AV_PIX_FMT_RGB32, SWS_FAST_BILINEAR,
									   nullptr, nullptr, nullptr);

	if (!m_yuv_to_rgba_ctx) {
		LibvideoLog() << "Couldn't initialize sw scaler\n";
	}

	m_dest_linesize[0] = width * 4;
}

YUVtoRGBAConvertor::~YUVtoRGBAConvertor() { sws_freeContext(m_yuv_to_rgba_ctx); }

std::vector<uint8_t> YUVtoRGBAConvertor::yuv_to_rgba(const struct AVFrame *frame) {
	uint8_t *data[1] = {(uint8_t *)m_rgba_decoded_data.data()};

	sws_scale(m_yuv_to_rgba_ctx, frame->data, frame->linesize, 0, frame->height, data, m_dest_linesize);

	return m_rgba_decoded_data;
}
} // namespace tomsksoft::libvideo