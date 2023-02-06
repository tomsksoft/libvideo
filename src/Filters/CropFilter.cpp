extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
}

#include <string>

#include "Logs.h"

#include "CropFilter.h"

namespace tomsksoft::libvideo {

CropFilter::CropFilter(int initial_width, int cropped_width, int cropped_height, int x_start_pos, int y_start_pos)
	: m_cropped_width(cropped_width), m_cropped_height(cropped_height), m_x_start_pos(x_start_pos), m_y_start_pos(y_start_pos) {
	m_cropped_data.insert(m_cropped_data.begin(), m_cropped_width * m_cropped_height * 4, 0);

	// RGBA
	m_start_offset_rgba = m_x_start_pos * m_y_start_pos * 4;
	m_new_line_offset_rgba = (initial_width - m_cropped_width) * 4 * 2;

	// YUV
	cropped_frame = av_frame_alloc();
	if (!cropped_frame) {
		LibvideoLog() << "Could not allocate video frame\n";
	}

	cropped_frame->format = AV_PIX_FMT_YUV420P;
	cropped_frame->width = cropped_width;
	cropped_frame->height = cropped_height;

	cropped_frame->linesize[0] = cropped_width;
	cropped_frame->linesize[1] = cropped_width / 2;
	cropped_frame->linesize[2] = cropped_width / 2;

	cropped_frame->data[0] = (uint8_t *)av_mallocz(cropped_width * cropped_height);
	cropped_frame->data[1] = (uint8_t *)av_mallocz(cropped_width * cropped_height / 4);
	cropped_frame->data[2] = (uint8_t *)av_mallocz(cropped_width * cropped_height / 4);

	m_start_offset_y = m_x_start_pos * m_y_start_pos;
	m_new_line_offset_y = (initial_width - m_cropped_width) * 2;

	m_start_offset_u_v = m_x_start_pos * m_y_start_pos / 2;
	m_new_line_offset_u_v = (initial_width - m_cropped_width);
}

std::vector<uint8_t> CropFilter::filtering_rgba(const std::vector<uint8_t> &initial_data) {
	unsigned int offset = m_start_offset_rgba;
	unsigned int push_offset = 0;
	const int copied_data_size = m_cropped_width * 4;

	for (int i = 0; i < m_cropped_height; i++) {
		memcpy(m_cropped_data.data() + push_offset, initial_data.data() + offset, copied_data_size);
		push_offset += copied_data_size;
		offset += m_new_line_offset_rgba;
	}
	return m_cropped_data;
}

AVFrame *CropFilter::filtering_yuv(struct AVFrame *frame) {

	cropped_frame->pts = frame->pts;

	unsigned int offset_y = m_start_offset_y;
	unsigned int push_offset_y = 0;
	const int copied_data_size_y = m_cropped_width;

	for (int i = 0; i < m_cropped_height; i++) {
		memcpy(cropped_frame->data[0] + push_offset_y, frame->data[0] + offset_y, copied_data_size_y);
		push_offset_y += copied_data_size_y;
		offset_y += m_new_line_offset_y;
	}

	unsigned int offset_u_v = m_start_offset_u_v;
	unsigned int push_offset_u_v = 0;
	const int copied_data_size_u_v = m_cropped_width / 2;

	for (int i = 0; i < m_cropped_height / 2; i++) {
		memcpy(cropped_frame->data[1] + push_offset_u_v, frame->data[1] + offset_u_v, copied_data_size_u_v);
		memcpy(cropped_frame->data[2] + push_offset_u_v, frame->data[2] + offset_u_v, copied_data_size_u_v);
		push_offset_u_v += copied_data_size_u_v;
		offset_u_v += m_new_line_offset_u_v;
	}

	return cropped_frame;
}

} // namespace tomsksoft::libvideo
