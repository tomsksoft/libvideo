/**
 * SWDecoder for raw RGBA stream
 *
 * Example of usage SWDecoder in test_package/LibvideoExample.cpp.
 *
 * This class implements software decoding.
 */

#ifndef LIBVIDEO_SW_DECODER_H
#define LIBVIDEO_SW_DECODER_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <chrono>
#include <memory>

#include "CropFilter.h"
#include "GeneralUtilities.h"
#include "ScaleFilter.h"

#include "DecoderParams.h"
#include "FilterParams.h"
#include "YUVtoRGBA.h"
#include "IDecoder.h"
#include "Logs.h"

#ifdef WIN32
#include "WindowOutputManagerSW.h"
#endif

namespace tomsksoft::libvideo {

class SWDecoder final : public IDecoder {
  public:
	explicit SWDecoder(DecoderParams params);
	~SWDecoder();

	void decode(std::vector<uint8_t> &encoded_data) override;
	void add_output(std::function<void(std::vector<uint8_t> &)> &&out_callback) override;
#ifdef WIN32
	void add_output(HWND hwnd) override;
#endif
	void add_filter(const FilterParams &filter_params) override;
	void set_logs_active(bool enable_logs) override;

  private:
	DecoderParams m_params;
	std::unique_ptr<YUVtoRGBAConvertor> m_convertor;
	int m_filtered_width = 0;
	int m_filtered_height = 0;
#ifdef WIN32
	WindowOutputManagerSW m_wnd_manager;
#endif

	const AVCodec *codec = nullptr;
	AVCodecParserContext *parser_ctx = nullptr;
	AVCodecContext *codec_ctx = nullptr;
	AVFrame *frame = nullptr;
	AVFrame *filtered_frame = nullptr;
	AVPacket *pkt = nullptr;

	std::vector<uint8_t> m_decoded_data;

	std::vector<std::shared_ptr<IFilter>> m_rgba_filters_list;
	std::vector<std::shared_ptr<IFilter>> m_yuv_filters_list;

	std::vector<std::function<void(std::vector<uint8_t> &)>> m_out_data_callbacks;

	bool check_existed_filters(std::vector<std::shared_ptr<IFilter>> &filters_list);
	void add_crop_filter(std::vector<std::shared_ptr<IFilter>> &filters_list, const FilterParams &filter_params);
	void add_scale_filter(std::vector<std::shared_ptr<IFilter>> &filters_list, const FilterParams &filter_params);

	void parse(std::vector<uint8_t> &encoded_data);
	void decode_data();
	void postprocessing();

	void initialize_extradata();

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

#endif // LIBVIDEO_SW_DECODER_H