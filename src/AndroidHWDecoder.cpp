extern "C" {
#include <libavcodec/mediacodec.h>
}

#include "AndroidHWDecoder.h"
#include "FilterParams.h"

namespace tomsksoft::libvideo {

AndroidHWDecoder::AndroidHWDecoder(const DecoderParams params, void *android_surface) : m_params(std::move(params)) {
	av_log_set_level(32);
	av_log_set_callback(android_log_callback);

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

	codec = avcodec_find_decoder_by_name("h264_mediacodec");
	if (!codec) {
		LibvideoLog() << "h264_mediacodec decoder not found\n";
		return;
	}

	codec_ctx = avcodec_alloc_context3(codec);
	if (!codec_ctx) {
		LibvideoLog() << "Could not allocate video codec context\n";
		return;
	}

	ret = av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_MEDIACODEC, NULL, NULL, 0);
	if (ret < 0) {
		LibvideoLog() << "Failed to create specified HW device";
		return;
	}

	ret = av_hwdevice_ctx_init(hw_device_ctx);
	if (ret < 0) {
		LibvideoLog() << "Failed to init specified HW device";
		return;
	}
	codec_ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);

	codec_ctx->pix_fmt = AV_PIX_FMT_MEDIACODEC;

	AVMediaCodecContext *android_ctx = av_mediacodec_alloc_context();

	ret = av_mediacodec_default_init(codec_ctx, android_ctx, android_surface);
	if (ret < 0) {
		LibvideoLog() << "Could not init mediacodec codec";
		return;
	}

	if (m_params.extradata.size()) {
		av_freep(&codec_ctx->extradata);
		codec_ctx->extradata = (uint8_t *)av_mallocz(m_params.extradata.size() + AV_INPUT_BUFFER_PADDING_SIZE);
		memcpy(codec_ctx->extradata, m_params.extradata.data(), m_params.extradata.size());
		codec_ctx->extradata_size = m_params.extradata.size();

		ret = avcodec_open2(codec_ctx, codec, nullptr);
		if (ret < 0) {
			LibvideoLog() << "Could not open codec\n";
			return;
		}

		m_codec_inited = true;
	} else
		LibvideoLog() << "No inputed extradata (SPS, PPS) by user. Trying find extradata (SPS, PPS) within a stream";

	m_current_frame_info.width = m_params.width;
	m_current_frame_info.height = m_params.height;
}

AndroidHWDecoder::~AndroidHWDecoder() {
	av_parser_close(parser_ctx);
	avcodec_free_context(&codec_ctx);
	av_frame_free(&frame);
	av_packet_free(&pkt);
	av_mediacodec_default_free(codec_ctx);
	av_buffer_unref(&hw_device_ctx);
}

void AndroidHWDecoder::decode(std::vector<uint8_t> &encoded_data) {
	encoded_data.insert(encoded_data.end(), AV_INPUT_BUFFER_PADDING_SIZE, 0);
	parse(encoded_data);
}

void AndroidHWDecoder::add_output(std::function<void(std::vector<uint8_t> &)> &&out_callback) {
	out_callback = [](std::vector<uint8_t> &data) { data = {}; };
	LibvideoLog() << "Can't add output to Android HW decoder\n";
}

void AndroidHWDecoder::add_filter(const FilterParams &filter_params) {
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

void AndroidHWDecoder::set_logs_active(bool enable_logs) { m_show_logs = enable_logs; }

void AndroidHWDecoder::parse(std::vector<uint8_t> &encoded_data) {
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

		if (pkt->size && !m_codec_inited)
			try_find_extradata_within_stream();
		else if (pkt->size && m_codec_inited)
			decode_data();
	}
}

void AndroidHWDecoder::try_find_extradata_within_stream() {

	int first_nalu_start_code_pos = -1;
	int second_nalu_start_code_pos = -1;
	int third_nalu_start_code_pos = -1;

	for (int i = 0; i < pkt->size - m_eh.nalu_sc4b_size; i++) {
		if (memcmp(m_eh.nalu_start_code_4_bytes, pkt->data + i, m_eh.nalu_sc4b_size) == 0) {
			first_nalu_start_code_pos = i;
			break;
		}
	}

	if (first_nalu_start_code_pos == -1) {
		LibvideoLog() << "No first NALU Start Code inside current packet (for SPS)";
		return;
	}

	if (*(pkt->data + first_nalu_start_code_pos + m_eh.nalu_sc4b_size) != m_eh.first_sps_byte) {
		LibvideoLog() << "No SPS data inside current packet";
		return;
	}

	for (int i = first_nalu_start_code_pos + m_eh.nalu_sc4b_size; i < pkt->size - m_eh.nalu_sc4b_size; i++) {
		if (memcmp(m_eh.nalu_start_code_4_bytes, pkt->data + i, m_eh.nalu_sc4b_size) == 0) {
			second_nalu_start_code_pos = i;
			break;
		}
	}

	if (second_nalu_start_code_pos == -1) {
		LibvideoLog() << "No second NALU Start Code inside current packet (for PPS)";
		return;
	}

	if (*(pkt->data + second_nalu_start_code_pos + m_eh.nalu_sc4b_size) != m_eh.first_pps_byte) {
		LibvideoLog() << "No PPS data inside current packet";
		return;
	}

	for (int i = second_nalu_start_code_pos + m_eh.nalu_sc4b_size; i < pkt->size - m_eh.nalu_sc3b_size; i++) {
		if (memcmp(m_eh.nalu_start_code_3_bytes, pkt->data + i, m_eh.nalu_sc3b_size) == 0) {
			third_nalu_start_code_pos = i;
			break;
		}
	}

	if (third_nalu_start_code_pos == -1) {
		LibvideoLog() << "No third NALU Start Code inside current packet (for end of PPS)";
		return;
	}

	const int sps_size = second_nalu_start_code_pos - first_nalu_start_code_pos;
	const int pps_size = third_nalu_start_code_pos - second_nalu_start_code_pos;

	av_freep(&codec_ctx->extradata);
	codec_ctx->extradata = (uint8_t *)av_mallocz(sps_size + pps_size + AV_INPUT_BUFFER_PADDING_SIZE);
	memcpy(codec_ctx->extradata, pkt->data + first_nalu_start_code_pos, sps_size);
	memcpy(codec_ctx->extradata + sps_size, pkt->data + second_nalu_start_code_pos, pps_size);
	codec_ctx->extradata_size = sps_size + pps_size;

	ret = avcodec_open2(codec_ctx, codec, nullptr);
	if (ret < 0) {
		LibvideoLog() << "Could not open codec: " << av_make_error_string(errBuf, sizeof(errBuf), ret);
		return;
	}

	LibvideoLog() << "Decoder initialized by stream";

	m_codec_inited = true;
}

void AndroidHWDecoder::decode_data() {
	ret = avcodec_send_packet(codec_ctx, pkt);
	if (ret < 0 && ret != AVERROR(EAGAIN)) {
		LibvideoLog() << "Error sending a packet for decoding";
		return;
	}
	ret = 0;

	while (ret >= 0) {
		ret = avcodec_receive_frame(codec_ctx, frame);
		if (ret == AVERROR(EAGAIN)) {
			ret = avcodec_send_packet(codec_ctx, pkt);
			if (ret < 0) {
				LibvideoLog() << "Error sending a packet for decoding: " << av_make_error_string(errBuf, sizeof(errBuf), ret);
			}
			continue;
		} else {
			if (ret == AVERROR_EOF) {
				LibvideoLog() << "End of file: " << av_make_error_string(errBuf, sizeof(errBuf), ret);
			}
		}

		if (frame->data[3]) {
			av_mediacodec_release_buffer(reinterpret_cast<AVMediaCodecBuffer *>(frame->data[3]), 1);
			av_frame_unref(frame);

			if (m_show_logs) {
				collect_stream_info();
				show_decoder_info(m_params.width, m_params.height, codec_ctx->bit_rate, m_current_frame_info.framerate,
								  codec_ctx->frame_number);
			}
		}

		break;
	}
}

void AndroidHWDecoder::show_decoder_info(int width, int height, int bitrate, int framerate, int dec_pts) {
	LibvideoLog() << "Decoder frame " << std::to_string(dec_pts) << " log: \n"
				  << "resolution: " << std::to_string(width) << "x" << std::to_string(height) << "\n"
				  << "bitrate: " << std::to_string(bitrate) << "\n"
				  << "framerate: " << std::to_string(framerate) << "\n\n";
}

void AndroidHWDecoder::collect_stream_info() {
	m_current_frame_info.bitrate = codec_ctx->bit_rate;
	m_current_frame_info.framerate = calc_framerate();
	m_current_frame_info.dec_pts = codec_ctx->frame_number;
}

int AndroidHWDecoder::calc_framerate() {
	second_time_point = std::chrono::steady_clock::now();

	auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(second_time_point - first_time_point);

	first_time_point = second_time_point;

	return (int)(1 / time_span.count());
}

} // namespace tomsksoft::libvideo