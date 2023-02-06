extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

#include "YUVtoNV12.h"

#include "Logs.h"

namespace tomsksoft::libvideo {
YUVtoNV12Convertor::YUVtoNV12Convertor(int width, int height) {
	m_yuv_to_nv12_ctx = sws_getContext(width, height, AV_PIX_FMT_YUV420P, width, height, AV_PIX_FMT_NV12, SWS_FAST_BILINEAR,
									   nullptr, nullptr, nullptr);

	if (!m_yuv_to_nv12_ctx) {
		LibvideoLog() << "Couldn't initialize sw scaler\n";
	}

	m_upos = width * height;
	m_vpos = m_upos + m_upos / 4;

	m_src_linesize[0] = width;
	m_src_linesize[1] = width / 2;
	m_src_linesize[2] = width / 2;
}

YUVtoNV12Convertor::~YUVtoNV12Convertor() { sws_freeContext(m_yuv_to_nv12_ctx); }

void YUVtoNV12Convertor::raw_yuv_to_nv12frame(struct AVFrame *dst_nv12_frame, const std::vector<uint8_t> &yuv_data) {
	const uint8_t *src_data[3] = {yuv_data.data(), yuv_data.data() + m_upos, yuv_data.data() + m_vpos};

	sws_scale(m_yuv_to_nv12_ctx, src_data, m_src_linesize, 0, dst_nv12_frame->height, dst_nv12_frame->data,
			  dst_nv12_frame->linesize);
}
} // namespace tomsksoft::libvideo