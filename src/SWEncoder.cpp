#include "SWEncoder.h"

namespace tomsksoft::libvideo {

SWEncoder::SWEncoder(EncoderParams params)
	: m_params(params), m_convertor(std::make_unique<RGBAtoYUVConvertor>(m_params.width, m_params.height)),
	  m_filtered_width(m_params.width), m_filtered_height(m_params.height) {
	codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!codec) {
		LibvideoLog() << "Codec not found\n";
		return;
	}

	codec_ctx = avcodec_alloc_context3(codec);
	if (!codec_ctx) {
		LibvideoLog() << "Could not allocate video codec context\n";
		return;
	}

	codec_ctx->bit_rate = m_params.bit_rate;
	codec_ctx->time_base = {1, m_params.framerate + 1};
	codec_ctx->framerate = {m_params.framerate + 1, 1};

	codec_ctx->gop_size = 0;

	codec_ctx->width = m_params.width;
	codec_ctx->height = m_params.height;

	codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

	if (codec->id == AV_CODEC_ID_H264)
		av_opt_set(codec_ctx->priv_data, "preset", Utilities::libvideo_h264preset_to_string(m_params.preset), 0);

	ret = avcodec_open2(codec_ctx, codec, nullptr);
	if (ret < 0) {
		LibvideoLog() << "Could not open codec: " << av_make_error_string(errBuf, sizeof(errBuf), ret) << "\n";
		return;
	}

	frame = av_frame_alloc();
	if (!frame) {
		LibvideoLog() << "Could not allocate video frame\n";
		return;
	}
	frame->format = codec_ctx->pix_fmt;
	frame->width = codec_ctx->width;
	frame->height = codec_ctx->height;
	frame->pts = 0;

	unfiltered_frame = frame;

	pkt = av_packet_alloc();
	if (!pkt) {
		LibvideoLog() << "Could not allocate packet\n";
		return;
	}

	ret = av_frame_get_buffer(frame, 0);
	if (ret < 0) {
		LibvideoLog() << "Could not allocate the video frame data: " << av_make_error_string(errBuf, sizeof(errBuf), ret) << "\n";
		return;
	}

	ret = av_frame_make_writable(frame);
	if (ret < 0) {
		LibvideoLog() << "Could not make frame writable: " << av_make_error_string(errBuf, sizeof(errBuf), ret) << "\n";
		return;
	}
}

SWEncoder::~SWEncoder() {
	avcodec_free_context(&codec_ctx);
	av_frame_free(&frame);
	av_packet_free(&pkt);
}

void SWEncoder::encode_frame(std::vector<uint8_t> &rgba_data) {
	preprocessing(rgba_data);

	std::vector<uint8_t> encoded_data;

	frame->pts++;

	ret = avcodec_send_frame(codec_ctx, frame);
	if (ret < 0) {
		LibvideoLog() << "Error sending a frame for encoding: " << av_make_error_string(errBuf, sizeof(errBuf), ret) << "\n";
	}

	while (ret >= 0) {
		ret = avcodec_receive_packet(codec_ctx, pkt);
		if (ret == AVERROR(EAGAIN)) {
			ret = avcodec_send_frame(codec_ctx, frame);
			if (ret < 0) {
				LibvideoLog() << "Error sending a frame for encoding: " << av_make_error_string(errBuf, sizeof(errBuf), ret)
							  << "\n";
			}
			continue;
		} else if (ret == AVERROR_EOF) {
			LibvideoLog() << "End of file: " << av_make_error_string(errBuf, sizeof(errBuf), ret) << "\n";
			break;
		} else if (ret < 0) {
			LibvideoLog() << "Error during encoding\n";
		}

		if (m_show_logs) {
			const int initial_size = m_filtered_width * m_filtered_height * 4;
			show_encoder_info(initial_size, pkt->size, codec_ctx->frame_number);
		}

		encoded_data.insert(encoded_data.begin(), pkt->data, pkt->data + pkt->size);

		collect_stream_info();

		av_packet_unref(pkt);
		break;
	}

	for (auto &callback : m_out_data_callbacks)
		callback(encoded_data);
}

void SWEncoder::preprocessing(std::vector<uint8_t> &rgba_data) {
	for (const auto &filter : m_rgba_filters_list)
		rgba_data = filter->filtering_rgba(rgba_data);

	std::vector<uint8_t> yuv_data = m_convertor->rgba_to_yuv(rgba_data);

	Utilities::raw_yuv_to_frame(unfiltered_frame, yuv_data);

	for (const auto &filter : m_yuv_filters_list)
		frame = filter->filtering_yuv(unfiltered_frame);
}

bool SWEncoder::check_existed_filters(std::vector<std::shared_ptr<IFilter>> &filters_list) {
	for (const auto &filter : filters_list) {
		if (dynamic_cast<CropFilter *>(filter.get())) {
			LibvideoLog() << "Crop filter already added\n";
			return true;
		}

		if (dynamic_cast<ScaleFilter *>(filter.get())) {
			LibvideoLog() << "Scale filter already added\n";
			return true;
		}
	}

	return false;
}

void SWEncoder::add_crop_filter(std::vector<std::shared_ptr<IFilter>> &filters_list, const FilterParams &filter_params) {
	float scale_coef = 1.f;

	ScaleFilter *scale_filter = nullptr;
	for (const auto &filter : m_rgba_filters_list) {
		scale_filter = dynamic_cast<ScaleFilter *>(filter.get());
	}

	for (const auto &filter : m_yuv_filters_list) {
		scale_filter = dynamic_cast<ScaleFilter *>(filter.get());
	}

	if (scale_filter)
		scale_coef = scale_filter->get_scale_coef();

	const int initial_width = (int)(scale_coef * m_params.width);

	filters_list.emplace_back(std::make_shared<CropFilter>(initial_width, filter_params.crop_params.cropped_width,
														   filter_params.crop_params.cropped_height,
														   filter_params.crop_params.x_start, filter_params.crop_params.y_start));

	m_filtered_width = filter_params.crop_params.cropped_width;
	m_filtered_height = filter_params.crop_params.cropped_height;
}

void SWEncoder::add_scale_filter(std::vector<std::shared_ptr<IFilter>> &filters_list, const FilterParams &filter_params) {
	int initial_width = m_params.width;
	int initial_height = m_params.height;

	CropFilter *crop_filter = nullptr;
	for (const auto &filter : m_rgba_filters_list) {
		crop_filter = dynamic_cast<CropFilter *>(filter.get());
	}

	if (crop_filter) {
		initial_width = crop_filter->get_width();
		initial_height = crop_filter->get_height();
	}

	filters_list.emplace_back(
		std::make_shared<ScaleFilter>(initial_width, initial_height, filter_params.scale_params.scale_coef));

	m_filtered_width = (int)(filter_params.scale_params.scale_coef * initial_width);
	m_filtered_height = (int)(filter_params.scale_params.scale_coef * initial_height);
}

void SWEncoder::add_filter(const FilterParams &filter_params) {

	if (check_existed_filters(m_rgba_filters_list))
		return;

	if (check_existed_filters(m_yuv_filters_list))
		return;

	if (filter_params.color_space == ColorSpace::RGBA) {
		if (filter_params.filter_type == FilterType::Crop)
			add_crop_filter(m_rgba_filters_list, filter_params);

		if (filter_params.filter_type == FilterType::Scale)
			add_scale_filter(m_rgba_filters_list, filter_params);
	}

	if (filter_params.color_space == ColorSpace::YUV) {
		if (filter_params.filter_type == FilterType::Crop)
			add_crop_filter(m_yuv_filters_list, filter_params);

		if (filter_params.filter_type == FilterType::Scale)
			add_scale_filter(m_yuv_filters_list, filter_params);
	}

	change_configuration(filter_params);
}

void SWEncoder::collect_stream_info() {
	const int initial_size = m_params.width * m_params.height * 4;

	m_current_frame_info.initial_size = initial_size;
	m_current_frame_info.pkt_size = pkt->size;
	m_current_frame_info.enc_pts = codec_ctx->frame_number;
}

void SWEncoder::change_configuration(const FilterParams &filter_params) {
	avcodec_free_context(&codec_ctx);
	av_frame_free(&frame);

	codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!codec) {
		LibvideoLog() << "Codec not found\n";
		return;
	}

	codec_ctx = avcodec_alloc_context3(codec);
	if (!codec_ctx) {
		LibvideoLog() << "Could not allocate video codec context\n";
		return;
	}

	codec_ctx->bit_rate = m_filtered_width * m_filtered_height * 4 * 8;
	codec_ctx->time_base = {1, m_params.framerate + 1};
	codec_ctx->framerate = {m_params.framerate + 1, 1};

	codec_ctx->width = m_filtered_width;
	codec_ctx->height = m_filtered_height;

	codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

	if (codec->id == AV_CODEC_ID_H264)
		av_opt_set(codec_ctx->priv_data, "preset", Utilities::libvideo_h264preset_to_string(m_params.preset), 0);

	ret = avcodec_open2(codec_ctx, codec, nullptr);
	if (ret < 0) {
		LibvideoLog() << "Could not open codec: " << av_make_error_string(errBuf, sizeof(errBuf), ret) << "\n";
		return;
	}

	frame = av_frame_alloc();
	if (!frame) {
		LibvideoLog() << "Could not allocate video frame\n";
		return;
	}
	frame->format = codec_ctx->pix_fmt;

	if (filter_params.color_space == ColorSpace::RGBA) {
		frame->width = m_filtered_width;
		frame->height = m_filtered_height;
	}

	if (filter_params.color_space == ColorSpace::YUV) {
		frame->width = m_params.width;
		frame->height = m_params.height;
	}
	frame->pts = 0;

	unfiltered_frame = frame;

	ret = av_frame_get_buffer(frame, 0);
	if (ret < 0) {
		LibvideoLog() << "Could not allocate the video frame data: " << av_make_error_string(errBuf, sizeof(errBuf), ret) << "\n";
		return;
	}

	ret = av_frame_make_writable(frame);
	if (ret < 0) {
		LibvideoLog() << "Could not make frame writable: " << av_make_error_string(errBuf, sizeof(errBuf), ret) << "\n";
		return;
	}

	m_convertor = std::make_unique<RGBAtoYUVConvertor>(frame->width, frame->height);
}

void SWEncoder::add_output(std::function<void(std::vector<uint8_t> &)> &&out_callback) {
	m_out_data_callbacks.emplace_back(std::move(out_callback));
}

void SWEncoder::set_logs_active(bool enable_logs) { m_show_logs = enable_logs; }

void SWEncoder::show_encoder_info(int initial_size, int pkt_size, int enc_pts) {
	LibvideoLog() << "Encoder frame " << std::to_string(enc_pts) << " log: \n"
				  << "frame size: " << std::to_string(initial_size) << "\n"
				  << "encoded size: " << std::to_string(pkt_size) << "\n\n";
}

} // namespace tomsksoft::libvideo