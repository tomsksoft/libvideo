/**
 * Android version of Decoder for raw RGBA stream
 *
 * Example of usage AndroidHWDecoder:
 * AndroidSDK\hw_decode\src\main\java\com\example\hwcameratest\MainActivity.java.
 *
 * This class implements hardware decoding.
 */

#ifndef LIBVIDEO_ANDROID_HW_DECODER_H
#define LIBVIDEO_ANDROID_HW_DECODER_H

#include <chrono>

#include "DecoderParams.h"
#include "IDecoder.h"
#include "Logs.h"

namespace tomsksoft::libvideo {

struct H264ExtradataHeaders {
	const uint8_t nalu_start_code_4_bytes[4] = {0x00, 0x00, 0x00, 0x01};
	const uint8_t nalu_start_code_3_bytes[3] = {0x00, 0x00, 0x01};
	const int nalu_sc4b_size = sizeof(nalu_start_code_4_bytes); // nalu_start_code_4_bytes_size
	const int nalu_sc3b_size = sizeof(nalu_start_code_3_bytes); // nalu_start_code_3_bytes_size

	const uint8_t first_sps_byte = 0x67;

	const uint8_t first_pps_byte = 0x68;
};

class AndroidHWDecoder final : public IDecoder {
  public:
	explicit AndroidHWDecoder(const DecoderParams params, void *android_surface);
	~AndroidHWDecoder();

	void decode(std::vector<uint8_t> &encoded_data) override;
	void add_output(std::function<void(std::vector<uint8_t> &)> &&out_callback) override;
	void add_filter(const FilterParams &filter_params) override;
	void set_logs_active(bool enable_logs) override;

  private:
	DecoderParams m_params;

	const AVCodec *codec = nullptr;
	AVCodecParserContext *parser_ctx = nullptr;
	AVCodecContext *codec_ctx = nullptr;
	AVFrame *frame = nullptr;
	AVPacket *pkt = nullptr;
	AVBufferRef *hw_device_ctx = nullptr;

	H264ExtradataHeaders m_eh;
	bool m_codec_inited = false;

	void parse(std::vector<uint8_t> &encoded_data);
	void try_find_extradata_within_stream();
	void decode_data();

	bool m_show_logs = true;
	void show_decoder_info(int width, int height, int bitrate, int framerate, int dec_pts);
	DecodedFrameInfo m_current_frame_info;
	void collect_stream_info();

	int calc_framerate();
	std::chrono::steady_clock::time_point first_time_point;
	std::chrono::steady_clock::time_point second_time_point;

	int ret = 0;
	char errBuf[64] = {0};
};

} // namespace tomsksoft::libvideo

#endif // LIBVIDEO_ANDROID_HW_DECODER_H