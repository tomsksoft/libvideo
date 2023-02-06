#include "FilterParams.h"
#include "WinHWEncoder.h"

namespace tomsksoft::libvideo {

WinHWEncoder::WinHWEncoder(EncoderParams params, enum class EncoderTypes hw_types)
	: m_params(params), m_RGBAtoYUVConvertor(std::make_unique<RGBAtoYUVConvertor>(m_params.width, m_params.height)),
	  m_YUVtoNV12Convertor(std::make_unique<YUVtoNV12Convertor>(m_params.width, m_params.height)) {

	init_hw_encoder(hw_types);

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

	sw_frame = av_frame_alloc();
	if (!sw_frame) {
		LibvideoLog() << "Could not allocate sw video frame\n";
		return;
	}

	sw_frame->format = AV_PIX_FMT_NV12;
	sw_frame->width = codec_ctx->width;
	sw_frame->height = codec_ctx->height;
	sw_frame->pts = 0;

	ret = av_frame_get_buffer(sw_frame, 0);
	if (ret < 0) {
		LibvideoLog() << "Could not allocate the video frame data: " << av_make_error_string(errBuf, sizeof(errBuf), ret) << "\n";
		return;
	}

	ret = av_frame_make_writable(sw_frame);
	if (ret < 0) {
		LibvideoLog() << "Could not make frame writable: " << av_make_error_string(errBuf, sizeof(errBuf), ret) << "\n";
		return;
	}

	pkt = av_packet_alloc();
	if (!pkt) {
		LibvideoLog() << "Could not allocate packet\n";
		return;
	}

	enum AVHWDeviceType device_type = AV_HWDEVICE_TYPE_D3D11VA;
	enum AVPixelFormat hw_pix_fmt = AV_PIX_FMT_D3D11;

	codec_ctx->pix_fmt = hw_pix_fmt;

	ret = av_hwdevice_ctx_create(&hw_device_ctx, device_type, nullptr, nullptr, 0);
	if (ret < 0) {
		LibvideoLog() << "Failed to create a HW device. Error code: " << av_make_error_string(errBuf, sizeof(errBuf), ret)
					  << "\n";
		return;
	}

	hw_frames_ref = av_hwframe_ctx_alloc(hw_device_ctx);
	if (!hw_frames_ref) {
		LibvideoLog() << "Failed to create HW frame context\n";
		return;
	}

	frames_ctx = (AVHWFramesContext *)(hw_frames_ref->data);
	frames_ctx->format = hw_pix_fmt;
	frames_ctx->sw_format = AV_PIX_FMT_NV12;
	frames_ctx->width = m_params.width;
	frames_ctx->height = m_params.height;

	ret = av_hwframe_ctx_init(hw_frames_ref);
	if (ret < 0) {
		LibvideoLog() << "Failed to initialize HW frame context. Error code: "
					  << av_make_error_string(errBuf, sizeof(errBuf), ret) << "\n";
		return;
	}

	codec_ctx->hw_frames_ctx = av_buffer_ref(hw_frames_ref);

	hw_frame = av_frame_alloc();
	if (!hw_frame) {
		LibvideoLog() << "Could not allocate hw video frame\n";
		return;
	}
	hw_frame->pts = 0;

	ret = av_hwframe_get_buffer(codec_ctx->hw_frames_ctx, hw_frame, 0);
	if (ret < 0) {
		LibvideoLog() << "Could not allocate hw frame buffer: " << av_make_error_string(errBuf, sizeof(errBuf), ret) << "\n";
		return;
	}

	ret = av_frame_make_writable(hw_frame);
	if (ret < 0) {
		LibvideoLog() << "Could not make frame writable: " << av_make_error_string(errBuf, sizeof(errBuf), ret) << "\n";
		return;
	}

	ret = avcodec_open2(codec_ctx, codec, nullptr);
	if (ret < 0) {
		LibvideoLog() << "Could not open codec: " << av_make_error_string(errBuf, sizeof(errBuf), ret) << "\n";
		return;
	}
}

WinHWEncoder::~WinHWEncoder() {
	avcodec_free_context(&codec_ctx);
	av_frame_free(&sw_frame);
	av_packet_free(&pkt);
	av_frame_free(&hw_frame);
	av_buffer_unref(&hw_device_ctx);
	av_buffer_unref(&hw_frames_ref);
}

void WinHWEncoder::init_hw_encoder(enum class EncoderTypes hw_types) {
	switch (hw_types) {
	case EncoderTypes::CUDA:
		codec = avcodec_find_encoder_by_name("h264_nvenc");
		break;

	case EncoderTypes::AMF:
		codec = avcodec_find_encoder_by_name("h264_amf");
		break;

	case EncoderTypes::QSV:
		codec = avcodec_find_encoder_by_name("h264_qsv");
		break;

	default:
		break;
	}

	if (!codec) {
		LibvideoLog() << "Codec not found\n";
		return;
	}
}

void WinHWEncoder::encode_frame(std::vector<uint8_t> &rgba_data) {
	preprocessing(rgba_data);

	std::vector<uint8_t> encoded_data;
	ret = av_hwframe_transfer_data(hw_frame, sw_frame, 0);
	if (ret < 0) {
		LibvideoLog() << "Error while transferring frame data to surface: " << av_make_error_string(errBuf, sizeof(errBuf), ret)
					  << "\n";
	}

	hw_frame->pts++;

	ret = avcodec_send_frame(codec_ctx, hw_frame);
	if (ret < 0) {
		LibvideoLog() << "Error sending a frame for encoding: " << av_make_error_string(errBuf, sizeof(errBuf), ret) << "\n";
	}

	while (ret >= 0) {
		ret = avcodec_receive_packet(codec_ctx, pkt);
		if (ret == AVERROR(EAGAIN)) {
			ret = avcodec_send_frame(codec_ctx, hw_frame);
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
			const int initial_size = m_params.width * m_params.height * 4;
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

void WinHWEncoder::preprocessing(std::vector<uint8_t> &rgba_data) {
	std::vector<uint8_t> yuv_data = m_RGBAtoYUVConvertor->rgba_to_yuv(rgba_data);

	m_YUVtoNV12Convertor->raw_yuv_to_nv12frame(sw_frame, yuv_data);
}

void WinHWEncoder::add_filter(const FilterParams &filter_params) {
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
	LibvideoLog() << "Can't add " + filter_type + " filter to HW encoder\n";
}

void WinHWEncoder::collect_stream_info() {
	const int initial_size = m_params.width * m_params.height * 4;

	m_current_frame_info.initial_size = initial_size;
	m_current_frame_info.pkt_size = pkt->size;
	m_current_frame_info.enc_pts = codec_ctx->frame_number;
}

void WinHWEncoder::add_output(std::function<void(std::vector<uint8_t> &)> &&out_callback) {
	m_out_data_callbacks.emplace_back(std::move(out_callback));
}

void WinHWEncoder::set_logs_active(bool enable_logs) { m_show_logs = enable_logs; }

void WinHWEncoder::show_encoder_info(int initial_size, int pkt_size, int enc_pts) {
	LibvideoLog() << "Encoder frame " << std::to_string(enc_pts) << " log: \n"
				  << "frame size: " << std::to_string(initial_size) << "\n"
				  << "encoded size: " << std::to_string(pkt_size) << "\n\n";
}

} // namespace tomsksoft::libvideo
