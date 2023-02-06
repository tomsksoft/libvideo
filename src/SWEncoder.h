/**
 * SWEncoder for raw RGBA stream
 *
 * Example of usage SWDecoder in test_package/LibvideoExample.cpp.
 *
 * This class implements software encoding.
 */

#ifndef LIBVIDEO_SW_ENCODER_H
#define LIBVIDEO_SW_ENCODER_H

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
#include "FilterParams.h"
#include "RGBAtoYUV.h"
#include "IEncoder.h"
#include "Logs.h"

namespace tomsksoft::libvideo {

class SWEncoder final : public IEncoder {
  public:
	explicit SWEncoder(EncoderParams params);
	~SWEncoder();

	void add_output(std::function<void(std::vector<uint8_t> &)> &&out_callback) override;
	void add_filter(const FilterParams &f_params) override;
	void encode_frame(std::vector<uint8_t> &rgba_video_data) override;
	void set_logs_active(bool enable_logs) override;

  private:
	EncoderParams m_params;
	std::unique_ptr<RGBAtoYUVConvertor> m_convertor;
	int m_filtered_width = 0;
	int m_filtered_height = 0;

	const AVCodec *codec = nullptr;
	AVCodecContext *codec_ctx = nullptr;
	AVFrame *frame = nullptr;
	AVFrame *unfiltered_frame = nullptr;
	AVPacket *pkt = nullptr;

	std::vector<std::shared_ptr<IFilter>> m_rgba_filters_list;
	std::vector<std::shared_ptr<IFilter>> m_yuv_filters_list;

	bool check_existed_filters(std::vector<std::shared_ptr<IFilter>> &filters_list);
	void add_crop_filter(std::vector<std::shared_ptr<IFilter>> &filters_list, const FilterParams &filter_params);
	void add_scale_filter(std::vector<std::shared_ptr<IFilter>> &filters_list, const FilterParams &filter_params);

	void preprocessing(std::vector<uint8_t> &rgba_data);
	void change_configuration(const FilterParams &filter_params);

	int ret = 0;
	char errBuf[64] = {0};

	bool m_show_logs = true;
	void show_encoder_info(int initial_size, int pkt_size, int enc_pts);

	EncodedFrameInfo m_current_frame_info;
	void collect_stream_info();

	std::vector<std::function<void(std::vector<uint8_t> &)>> m_out_data_callbacks;
};

} // namespace tomsksoft::libvideo

#endif // LIBVIDEO_SW_ENCODER_H