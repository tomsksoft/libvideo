#include "SWDecoder.h"

namespace tomsksoft::libvideo {

SWDecoder::SWDecoder(DecoderParams params)
	: m_params(params), m_convertor(std::make_unique<YUVtoRGBAConvertor>(m_params.width, m_params.height)),
	  m_filtered_width(m_params.width), m_filtered_height(m_params.height) {

	parser_ctx = av_parser_init(AV_CODEC_ID_H264);
	if (!parser_ctx) {
		LibvideoLog() << "Parser not found\n";
		return;
	}

	pkt = av_packet_alloc();
	if (!pkt) {
		LibvideoLog() << "Could not allocate packet\n";
		return;
	}

	frame = av_frame_alloc();
	if (!frame) {
		LibvideoLog() << "Could not allocate video frame\n";
		return;
	}
	filtered_frame = frame;

	codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!codec) {
		LibvideoLog() << "Codec not found\n";
		return;
	}

	codec_ctx = avcodec_alloc_context3(codec);
	if (!codec_ctx) {
		LibvideoLog() << "Could not allocate video cdsaodec context\n";
		return;
	}

#ifdef __ANDROID_API__
#if __ANDROID_API__ > 20
	if (m_params.extradata.size()) {
		av_freep(&codec_ctx->extradata);
		codec_ctx->extradata = (uint8_t *)av_mallocz(m_params.extradata.size() + AV_INPUT_BUFFER_PADDING_SIZE);
		memcpy(codec_ctx->extradata, m_params.extradata.data(), m_params.extradata.size());
		codec_ctx->extradata_size = m_params.extradata.size();
	}
#endif
#endif

	ret = avcodec_open2(codec_ctx, codec, nullptr);
	if (ret < 0) {
		LibvideoLog() << "Could not open codec\n";
		return;
	}
}

SWDecoder::~SWDecoder() {
	av_parser_close(parser_ctx);
	avcodec_free_context(&codec_ctx);
	av_frame_free(&frame);
	av_packet_free(&pkt);
}

void SWDecoder::decode(std::vector<uint8_t> &encoded_data) {
	encoded_data.insert(encoded_data.end(), AV_INPUT_BUFFER_PADDING_SIZE, 0);

	parse(encoded_data);

	for (auto &callback : m_out_data_callbacks)
		callback(m_decoded_data);

#ifdef WIN32
	m_wnd_manager.draw({&m_decoded_data, &m_current_frame_info, m_show_logs});
#endif
}

void SWDecoder::add_output(std::function<void(std::vector<uint8_t> &)> &&out_callback) {
	m_out_data_callbacks.emplace_back(std::move(out_callback));
}

#ifdef WIN32
void SWDecoder::add_output(HWND hwnd) {
	RECT rect;
	GetClientRect(hwnd, &rect);
	const int window_width = rect.right - rect.left;
	const int window_height = rect.bottom - rect.top;

	if ((window_width != m_filtered_width) || (window_height != m_filtered_height)) {
		LibvideoLog() << "The window dimensions do not match the image dimensions\n";
		return;
	}

	m_wnd_manager.initialize_window(hwnd);
}
#endif

bool SWDecoder::check_existed_filters(std::vector<std::shared_ptr<IFilter>> &filters_list) {
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
void SWDecoder::add_crop_filter(std::vector<std::shared_ptr<IFilter>> &filters_list, const FilterParams &filter_params) {
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

	const int initial_width = static_cast<int>(scale_coef * m_params.width);

	filters_list.emplace_back(std::make_shared<CropFilter>(initial_width, filter_params.crop_params.cropped_width,
														   filter_params.crop_params.cropped_height,
														   filter_params.crop_params.x_start, filter_params.crop_params.y_start));

	m_filtered_width = filter_params.crop_params.cropped_width;
	m_filtered_height = filter_params.crop_params.cropped_height;
}
void SWDecoder::add_scale_filter(std::vector<std::shared_ptr<IFilter>> &filters_list, const FilterParams &filter_params) {
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

	m_filtered_width = static_cast<int>(filter_params.scale_params.scale_coef * initial_width);
	m_filtered_height = static_cast<int>(filter_params.scale_params.scale_coef * initial_height);
}

void SWDecoder::add_filter(const FilterParams &filter_params) {
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

		m_convertor = std::make_unique<YUVtoRGBAConvertor>(m_filtered_width, m_filtered_height);
	}
}

void SWDecoder::set_logs_active(bool enable_logs) { m_show_logs = enable_logs; }

void SWDecoder::parse(std::vector<uint8_t> &encoded_data) {
	int data_size = static_cast<int>(encoded_data.size());

	uint8_t *data = encoded_data.data();

	bool data_end = false;

	while (!data_end) {
		data_end = (data_size) ? false : true;

		ret = av_parser_parse2(parser_ctx, codec_ctx, &pkt->data, &pkt->size, data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
		if (ret < 0) {
			LibvideoLog() << "Error while parsing\n";
			return;
		}

		data += ret;
		data_size -= ret;

		if (pkt->size)
			decode_data();
	}
}

void SWDecoder::decode_data() {
	ret = avcodec_send_packet(codec_ctx, pkt);
	if (ret < 0) {
		LibvideoLog() << "Error sending a packet for decoding\n";
		return;
	}

	while (ret >= 0) {
		ret = avcodec_receive_frame(codec_ctx, frame);
		if (ret == AVERROR(EAGAIN)) {
			ret = avcodec_send_packet(codec_ctx, pkt);
			if (ret < 0) {
				LibvideoLog() << "Error sending a packet for decoding: " << av_make_error_string(errBuf, sizeof(errBuf), ret)
							  << "\n";
			}
			continue;
		} else if (ret == AVERROR_EOF) {
			LibvideoLog() << "End of file: " << av_make_error_string(errBuf, sizeof(errBuf), ret) << "\n";
		}

		if (m_show_logs) {
			collect_stream_info();
			show_decoder_info(m_filtered_width, m_filtered_height, codec_ctx->bit_rate, m_current_frame_info.framerate,
							  codec_ctx->frame_number);
		}

		break;
	}

	postprocessing();
}

void SWDecoder::postprocessing() {
	if (m_params.width != frame->width || m_params.height != frame->height) {
		LibvideoLog() << "Decoder initialization sizes do not match decoded frame sizes\n";

		m_convertor = std::make_unique<YUVtoRGBAConvertor>(frame->width, frame->height);

		m_params.width = frame->width;
		m_params.height = frame->height;
	}

	for (const auto &filter : m_yuv_filters_list)
		filtered_frame = filter->filtering_yuv(frame);

	std::vector<uint8_t> temp_rgba = m_convertor->yuv_to_rgba(filtered_frame);

	for (const auto &filter : m_rgba_filters_list)
		temp_rgba = filter->filtering_rgba(temp_rgba);

	std::swap(m_decoded_data, temp_rgba);
}

void SWDecoder::show_decoder_info(int width, int height, int64_t bitrate, int framerate, int dec_pts) {
	LibvideoLog() << "Decoder frame " << std::to_string(dec_pts) << " log: \n"
				  << "resolution: " << std::to_string(width) << "x" << std::to_string(height) << "\n"
				  << "bitrate: " << std::to_string(bitrate) << "\n"
				  << "framerate: " << std::to_string(framerate) << "\n\n";
}

void SWDecoder::collect_stream_info() {
	const int framerate = calc_framerate();

	m_current_frame_info.width = m_params.width;
	m_current_frame_info.height = m_params.height;
	m_current_frame_info.bitrate = codec_ctx->bit_rate;
	m_current_frame_info.framerate = framerate;
	m_current_frame_info.dec_pts = codec_ctx->frame_number;
}

int SWDecoder::calc_framerate() {
	second_time_point = std::chrono::steady_clock::now();

	std::chrono::duration<double> time_span =
		std::chrono::duration_cast<std::chrono::duration<double>>(second_time_point - first_time_point);

	first_time_point = second_time_point;

	return static_cast<int>(1 / time_span.count());
}

} // namespace tomsksoft::libvideo