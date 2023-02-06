#include "WinHWDecoder.h"

namespace tomsksoft::libvideo {

WinHWDecoder::WinHWDecoder(DecoderParams params, enum class DecoderTypes hw_types)
	: m_params(params), m_convertor(std::make_unique<YUVtoRGBAConvertor>(m_params.width, m_params.height)), m_hw_types(hw_types) {

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

	codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!codec) {
		LibvideoLog() << "Codec not found\n";
		return;
	}

	codec_ctx = avcodec_alloc_context3(codec);
	if (!codec_ctx) {
		LibvideoLog() << "Could not allocate video codec context\n";
		return;
	}

	prepare_hardware_decoding(hw_types);

	ret = avcodec_open2(codec_ctx, codec, nullptr);
	if (ret < 0) {
		LibvideoLog() << "Could not open codec\n";
		return;
	}
}

WinHWDecoder::~WinHWDecoder() {
	av_parser_close(parser_ctx);
	avcodec_free_context(&codec_ctx);
	av_frame_free(&frame);
	av_packet_free(&pkt);
	av_frame_free(&sw_frame);
	av_buffer_unref(&hw_device_ctx);
}

void WinHWDecoder::prepare_hardware_decoding(enum DecoderTypes hw_types) {
	const enum AVHWDeviceType type = libvideoDecoderTypes_to_AVHWDeviceType(hw_types);

	sw_frame = av_frame_alloc();
	if (!sw_frame) {
		LibvideoLog() << "Could not allocate video frame\n";
		return;
	}

	for (int i = 0;; i++) {
		const AVCodecHWConfig *config = avcodec_get_hw_config(codec, i);
		if (!config) {
			LibvideoLog() << "Decoder " << codec->name << " does not support device type " << av_hwdevice_get_type_name(type)
						  << "\n";
			return;
		}
		if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX && config->device_type == type) {
			hw_pix_fmt = config->pix_fmt;
			break;
		}
	}

	ret = av_hwdevice_ctx_create(&hw_device_ctx, type, NULL, NULL, 0);
	if (ret < 0) {
		LibvideoLog() << "Failed to create specified HW device\n";
		return;
	}
	codec_ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
}

enum AVHWDeviceType WinHWDecoder::libvideoDecoderTypes_to_AVHWDeviceType(enum class DecoderTypes hw_types) {
	switch (hw_types) {
	case DecoderTypes::CUDA:
		return AVHWDeviceType::AV_HWDEVICE_TYPE_CUDA;

	case DecoderTypes::DX9:
		return AVHWDeviceType::AV_HWDEVICE_TYPE_DXVA2;

	case DecoderTypes::DX11:
		return AVHWDeviceType::AV_HWDEVICE_TYPE_D3D11VA;

	case DecoderTypes::QSV:
		return AVHWDeviceType::AV_HWDEVICE_TYPE_QSV;

	case DecoderTypes::OPENCL:
		return AVHWDeviceType::AV_HWDEVICE_TYPE_OPENCL;

	default:
		break;
	}

	return AVHWDeviceType::AV_HWDEVICE_TYPE_D3D11VA;
}

void WinHWDecoder::decode(std::vector<uint8_t> &encoded_data) {
	encoded_data.insert(encoded_data.end(), AV_INPUT_BUFFER_PADDING_SIZE, 0);

	parse(encoded_data);

	WinHWData data{nullptr, 0};
	data.texture = reinterpret_cast<void *>(frame->data[0]);
	memcpy(&data.texture_index, &frame->data[1], sizeof(data.texture_index));

	m_wnd_manager.draw(data);
}

void WinHWDecoder::add_output(std::function<void(std::vector<uint8_t> &)> &&out_callback) {
	m_out_data_callbacks.emplace_back(std::move(out_callback));
}

void WinHWDecoder::add_output(HWND hwnd) {
	if (m_hw_types == DecoderTypes::DX11)
		m_wnd_manager.initialize_window(hwnd);
	else
		LibvideoLog() << "Can't draw on window without DX11 decoder device\n";
}

void WinHWDecoder::add_filter(const FilterParams &filter_params) {
	std::string filter_type;
	switch (filter_params.filter_type) {
	case FilterType::Scale:
		filter_type = "scale";
		break;

	case FilterType::Crop:
		filter_type = "crop";
		break;

	default:
		break;
	}
	LibvideoLog() << "Can't add " + filter_type + " filter to HW decoder\n";
}

void WinHWDecoder::set_logs_active(bool enable_logs) { m_show_logs = enable_logs; }

void WinHWDecoder::parse(std::vector<uint8_t> &encoded_data) {
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

void WinHWDecoder::decode_data() {
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
			show_decoder_info(m_params.width, m_params.height, codec_ctx->bit_rate, m_current_frame_info.framerate,
							  codec_ctx->frame_number);
		}

		break;
	}
}

std::vector<uint8_t> WinHWDecoder::postprocessing() {
	std::vector<uint8_t> temp_rgba = m_convertor->yuv_to_rgba(tmp_frame);

	m_params.width = tmp_frame->width;
	m_params.height = tmp_frame->height;

	return temp_rgba;
}

void WinHWDecoder::retrieve_data_from_hw_frame() {
	if (frame->format == hw_pix_fmt) {
		// Retrieve data from GPU to CPU
		if ((ret = av_hwframe_transfer_data(sw_frame, frame, 0)) < 0) {
			LibvideoLog() << "Error transferring the data to system memory\n";
		}
		tmp_frame = sw_frame;
	} else {
		tmp_frame = frame;
	}
}

void WinHWDecoder::show_decoder_info(int width, int height, int64_t bitrate, int framerate, int dec_pts) {
	LibvideoLog() << "Decoder frame " << std::to_string(dec_pts) << " log: \n"
				  << "resolution: " << std::to_string(width) << "x" << std::to_string(height) << "\n"
				  << "bitrate: " << std::to_string(bitrate) << "\n"
				  << "framerate: " << std::to_string(framerate) << "\n\n";
}

void WinHWDecoder::collect_stream_info() {
	const int framerate = calc_framerate();

	m_current_frame_info.width = m_params.width;
	m_current_frame_info.height = m_params.height;
	m_current_frame_info.bitrate = codec_ctx->bit_rate;
	m_current_frame_info.framerate = framerate;
	m_current_frame_info.dec_pts = codec_ctx->frame_number;
}

int WinHWDecoder::calc_framerate() {
	second_time_point = std::chrono::steady_clock::now();

	std::chrono::duration<double> time_span =
		std::chrono::duration_cast<std::chrono::duration<double>>(second_time_point - first_time_point);

	first_time_point = second_time_point;

	return static_cast<int>(1 / time_span.count());
}

} // namespace tomsksoft::libvideo
