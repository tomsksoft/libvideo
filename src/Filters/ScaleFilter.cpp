extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include "Logs.h"

#include "ScaleFilter.h"

namespace tomsksoft::libvideo {

ScaleFilter::ScaleFilter(int initial_width, int initial_height, float scale_coef)
	: m_scale_coef(scale_coef), m_initial_height(initial_height) {
	m_scaled_width = static_cast<int>(initial_width * m_scale_coef);
	m_scaled_height = static_cast<int>(initial_height * m_scale_coef);

	// RGBA
	m_scaled_rgba_data.insert(m_scaled_rgba_data.begin(), m_scaled_width * m_scaled_height * 4, 0);
	m_scale_rgba_ctx = sws_getContext(initial_width, initial_height, AV_PIX_FMT_RGB32, m_scaled_width, m_scaled_height,
									  AV_PIX_FMT_RGB32, SWS_POINT, nullptr, nullptr, nullptr);
	if (!m_scale_rgba_ctx) {
		LibvideoLog() << "Couldn't initialize sw scaler\n";
	}

	m_dest_linesize[0] = m_scaled_width * 4;
	m_src_linesize[0] = initial_width * 4;

	// YUV
	m_scaled_frame = av_frame_alloc();
	if (!m_scaled_frame) {
		LibvideoLog() << "Could not allocate video frame\n";
	}

	m_scale_yuv_ctx = sws_getContext(initial_width, initial_height, AV_PIX_FMT_YUV420P, m_scaled_width, m_scaled_height,
									 AV_PIX_FMT_YUV420P, SWS_POINT, nullptr, nullptr, nullptr);
	if (!m_scale_yuv_ctx) {
		LibvideoLog() << "Couldn't initialize sw scaler\n";
	}
}

ScaleFilter::~ScaleFilter() {
	sws_freeContext(m_scale_rgba_ctx);
	sws_freeContext(m_scale_yuv_ctx);
}

std::vector<uint8_t> ScaleFilter::filtering_rgba(const std::vector<uint8_t> &initial_data) {

	uint8_t *src_data[1] = {(uint8_t *)initial_data.data()};
	uint8_t *dst_data[1] = {(uint8_t *)m_scaled_rgba_data.data()};

	sws_scale(m_scale_rgba_ctx, src_data, m_src_linesize, 0, m_initial_height, dst_data, m_dest_linesize);

	return m_scaled_rgba_data;
}

AVFrame *ScaleFilter::filtering_yuv(struct AVFrame *frame) {
	sws_scale_frame(m_scale_yuv_ctx, m_scaled_frame, frame);

	return m_scaled_frame;
}

} // namespace tomsksoft::libvideo
