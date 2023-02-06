/**
 * Windows version of Decoder for raw RGBA stream
 *
 * This class implements hardware encoding.
 */

#ifndef LIBVIDEO_WIN_HW_ENCODER_H
#define LIBVIDEO_WIN_HW_ENCODER_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

#include <memory>
#include <utility>

#include "CropFilter.h"
#include "GeneralUtilities.h"
#include "ScaleFilter.h"

#include "EncoderParams.h"
#include "RGBAtoYUV.h"
#include "YUVtoNV12.h"
#include "IEncoder.h"
#include "Logs.h"

namespace tomsksoft::libvideo {

class WinHWEncoder final : public IEncoder {
  public:
	explicit WinHWEncoder(EncoderParams params, enum class EncoderTypes hw_types);
	~WinHWEncoder();

	void add_output(std::function<void(std::vector<uint8_t> &)> &&out_callback) override;
	void add_filter(const FilterParams &f_params) override;
	void encode_frame(std::vector<uint8_t> &rgba_video_data) override;
	void set_logs_active(bool enable_logs) override;

  private:
	EncoderParams m_params;
	std::unique_ptr<RGBAtoYUVConvertor> m_RGBAtoYUVConvertor;
	std::unique_ptr<YUVtoNV12Convertor> m_YUVtoNV12Convertor;

	const AVCodec *codec = nullptr;
	AVCodecContext *codec_ctx = nullptr;
	AVFrame *sw_frame = nullptr;
	AVPacket *pkt = nullptr;

	AVFrame *hw_frame = nullptr;
	AVBufferRef *hw_device_ctx = nullptr;
	AVBufferRef *hw_frames_ref = nullptr;
	AVHWFramesContext *frames_ctx = nullptr;

	void preprocessing(std::vector<uint8_t> &rgba_data);

	void prepare_hardware_encoding();

	void init_hw_encoder(enum class EncoderTypes hw_types);

	int ret = 0;
	char errBuf[64] = {0};

	bool m_show_logs = true;
	void show_encoder_info(int initial_size, int pkt_size, int enc_pts);

	EncodedFrameInfo m_current_frame_info;
	void collect_stream_info();

	std::vector<std::function<void(std::vector<uint8_t> &)>> m_out_data_callbacks;
};

} // namespace tomsksoft::libvideo

#endif // LIBVIDEO_WIN_HW_ENCODER_H