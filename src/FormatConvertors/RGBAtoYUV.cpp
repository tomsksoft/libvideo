extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

#include "RGBAtoYUV.h"

#include "Logs.h"

namespace tomsksoft::libvideo {
RGBAtoYUVConvertor::RGBAtoYUVConvertor(int width, int height) : m_height(height) {
	m_yuv_data.insert(m_yuv_data.begin(), width * height * 3 / 2, 0);
	m_rgba_to_yuv_ctx = sws_getContext(width, height, AV_PIX_FMT_RGB32, width, height, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR,
									   nullptr, nullptr, nullptr);
	if (!m_rgba_to_yuv_ctx) {
		LibvideoLog() << "Couldn't initialize sw scaler\n";
	}

	m_upos = height * width;
	m_vpos = m_upos + m_upos / 4;

	m_dest_data[0] = m_yuv_data.data();
	m_dest_data[1] = m_yuv_data.data() + m_upos;
	m_dest_data[2] = m_yuv_data.data() + m_vpos;

	m_dest_linesize[0] = width;
	m_dest_linesize[1] = width / 2;
	m_dest_linesize[2] = width / 2;

	m_cur_linesize[0] = width * 4;
}

RGBAtoYUVConvertor::~RGBAtoYUVConvertor() { sws_freeContext(m_rgba_to_yuv_ctx); }

std::vector<uint8_t> RGBAtoYUVConvertor::rgba_to_yuv(const std::vector<uint8_t> &rgba_data) {
	const uint8_t *data[1] = {rgba_data.data()};

	sws_scale(m_rgba_to_yuv_ctx, data, m_cur_linesize, 0, m_height, m_dest_data, m_dest_linesize);

	return m_yuv_data;
}
} // namespace tomsksoft::libvideo