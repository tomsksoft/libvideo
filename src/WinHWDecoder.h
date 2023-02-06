/**
 * Windows version of Decoder for raw RGBA stream
 *
 * This class implements hardware decoding.
 */

#ifndef LIBVIDEO_WIN_HW_DECODER_H
#define LIBVIDEO_WIN_HW_DECODER_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <chrono>

#include "GeneralUtilities.h"

#include "DecoderParams.h"
#include "FilterParams.h"
#include "YUVtoRGBA.h"
#include "IDecoder.h"
#include "Logs.h"
#include "WindowOutputManagerHW.h"

namespace tomsksoft::libvideo {

class WinHWDecoder final : public IDecoder {
  public:
	explicit WinHWDecoder(DecoderParams params, enum class DecoderTypes hw_types);
	~WinHWDecoder();

	void decode(std::vector<uint8_t> &encoded_data) override;
	void add_output(std::function<void(std::vector<uint8_t> &)> &&out_callback) override;
	void add_output(HWND hwnd) override;
	void add_filter(const FilterParams &filter_params) override;
	void set_logs_active(bool enable_logs) override;

  private:
	DecoderParams m_params;
	std::unique_ptr<YUVtoRGBAConvertor> m_convertor;
	WindowOutputManagerHW m_wnd_manager;

	const AVCodec *codec = nullptr;
	AVCodecParserContext *parser_ctx = nullptr;
	AVCodecContext *codec_ctx = nullptr;
	AVFrame *frame = nullptr;
	AVPacket *pkt = nullptr;

	AVFrame *sw_frame = nullptr;
	AVFrame *tmp_frame = nullptr;
	AVBufferRef *hw_device_ctx = nullptr;
	enum AVPixelFormat hw_pix_fmt = AV_PIX_FMT_NONE;
	enum class DecoderTypes m_hw_types;

	std::vector<std::function<void(std::vector<uint8_t> &)>> m_out_data_callbacks;

	void prepare_hardware_decoding(enum class DecoderTypes hw_types);
	enum AVHWDeviceType libvideoDecoderTypes_to_AVHWDeviceType(enum class DecoderTypes hw_types);

	void parse(std::vector<uint8_t> &encoded_data);
	void decode_data();
	[[maybe_unused]] std::vector<uint8_t> postprocessing();
	[[maybe_unused]] void retrieve_data_from_hw_frame();

	bool m_show_logs = true;
	void show_decoder_info(int width, int height, int64_t bitrate, int framerate, int dec_pts);
	DecodedFrameInfo m_current_frame_info;
	void collect_stream_info();

	int calc_framerate();
	std::chrono::steady_clock::time_point first_time_point;
	std::chrono::steady_clock::time_point second_time_point;

	int ret = 0;
	char errBuf[64] = {0};
};

} // namespace tomsksoft::libvideo

#endif // LIBVIDEO_WIN_HW_DECODER_H
