extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}
#include <fstream>

#include "GeneralUtilities.h"

#include "Logs.h"

namespace tomsksoft::libvideo {

namespace Utilities {
void fill_by_raw_rgba_data(std::vector<uint8_t> &raw_data, const int width, const int height) {
	int start_offset = width * (height / 3) + (width / 3);
	const int end_offset = width * ((height / 3) * 2) + (width / 3) * 2;

	for (int i = 0; i < width * height; i += 1) {
		if (i == start_offset) {
			for (int j = 0; j < (width / 3); j++) {
				raw_data.insert(raw_data.end(), {0, 255, 0, 0});
			}
			i += (width / 3);
			start_offset = (start_offset + (width / 3) == end_offset) ? 0 : start_offset + width;
		}
		raw_data.insert(raw_data.end(), {255, 0, 0, 0});
	}
}

void fill_by_raw_SMPTE_rgba_data(std::vector<uint8_t> &raw_data, const int width, const int height) {
	const uint8_t colour_bar[16][4] = {
		{180, 180, 180, 0}, // White 75 %
		{180, 180, 16, 0},	// Yellow
		{16, 180, 180, 0},	// Cyan
		{16, 180, 16, 0},	// Green
		{180, 16, 180, 0},	// Magenta
		{180, 16, 16, 0},	// Red
		{16, 16, 180, 0},	// Blue
		{16, 16, 16, 0},	// Black
		{16, 70, 106, 0},	// Dark Blue
		{72, 16, 118, 0},	// Dark Purple
		{24, 24, 24, 0},	// Dark Grey (Left black rectangle)
		{10, 10, 10, 0},	// Darkest Grey
		{235, 235, 235, 0}, // White 100 %
		{31, 34, 36, 0},	// Right black rectangle
		{16, 16, 16, 0},	// Middle black rectangle
		{8, 8, 8, 0},		// Left black rectangle
	};

	int first_height_part = (int)(height * 0.67f);
	int column_width = width / 7;
	int offset = width - (column_width * 7);
	for (int y = 0; y < first_height_part; y++) {
		for (int x = 0; x < column_width; x++)
			raw_data.insert(raw_data.end(), colour_bar[0], colour_bar[0] + 4);

		for (int x = 0; x < column_width; x++)
			raw_data.insert(raw_data.end(), colour_bar[1], colour_bar[1] + 4);

		for (int x = 0; x < column_width; x++)
			raw_data.insert(raw_data.end(), colour_bar[2], colour_bar[2] + 4);

		for (int x = 0; x < column_width; x++)
			raw_data.insert(raw_data.end(), colour_bar[3], colour_bar[3] + 4);

		for (int x = 0; x < column_width; x++)
			raw_data.insert(raw_data.end(), colour_bar[4], colour_bar[4] + 4);

		for (int x = 0; x < column_width; x++)
			raw_data.insert(raw_data.end(), colour_bar[5], colour_bar[5] + 4);

		for (int x = 0; x < column_width + offset; x++)
			raw_data.insert(raw_data.end(), colour_bar[6], colour_bar[6] + 4);
	}

	int second_height_part = (int)(height * 0.1f);
	for (int y = 0; y < second_height_part; y++) {
		for (int x = 0; x < column_width; x++)
			raw_data.insert(raw_data.end(), colour_bar[6], colour_bar[6] + 4);

		for (int x = 0; x < column_width; x++)
			raw_data.insert(raw_data.end(), colour_bar[7], colour_bar[7] + 4);

		for (int x = 0; x < column_width; x++)
			raw_data.insert(raw_data.end(), colour_bar[4], colour_bar[4] + 4);

		for (int x = 0; x < column_width; x++)
			raw_data.insert(raw_data.end(), colour_bar[7], colour_bar[7] + 4);

		for (int x = 0; x < column_width; x++)
			raw_data.insert(raw_data.end(), colour_bar[2], colour_bar[2] + 4);

		for (int x = 0; x < column_width; x++)
			raw_data.insert(raw_data.end(), colour_bar[7], colour_bar[7] + 4);

		for (int x = 0; x < column_width + offset; x++)
			raw_data.insert(raw_data.end(), colour_bar[0], colour_bar[0] + 4);
	}

	int bottom_square_width = (column_width * 5) / 4;
	int black_lines_width = column_width / 3;
	int top_bottom_lines_offset = (column_width * 5) - (bottom_square_width * 4);
	int last_squere_width = width - (bottom_square_width * 4) - (black_lines_width * 3) - top_bottom_lines_offset;
	int third_height_part = height - first_height_part - second_height_part;
	for (int y = 0; y < third_height_part; y++) {
		for (int x = 0; x < bottom_square_width; x++)
			raw_data.insert(raw_data.end(), colour_bar[8], colour_bar[8] + 4);

		for (int x = 0; x < bottom_square_width; x++)
			raw_data.insert(raw_data.end(), colour_bar[12], colour_bar[12] + 4);

		for (int x = 0; x < bottom_square_width; x++)
			raw_data.insert(raw_data.end(), colour_bar[9], colour_bar[9] + 4);

		for (int x = 0; x < bottom_square_width + top_bottom_lines_offset; x++)
			raw_data.insert(raw_data.end(), colour_bar[7], colour_bar[7] + 4);

		for (int x = 0; x < black_lines_width; x++)
			raw_data.insert(raw_data.end(), colour_bar[15], colour_bar[15] + 4);

		for (int x = 0; x < black_lines_width; x++)
			raw_data.insert(raw_data.end(), colour_bar[7], colour_bar[7] + 4);

		for (int x = 0; x < black_lines_width; x++)
			raw_data.insert(raw_data.end(), colour_bar[13], colour_bar[13] + 4);

		for (int x = 0; x < last_squere_width; x++)
			raw_data.insert(raw_data.end(), colour_bar[7], colour_bar[7] + 4);
	}
}

#ifdef WIN32
void dump_image(const std::vector<uint8_t> &raw_data, const std::string &filename) {

	std::ofstream out(filename, std::ios_base::out, std::ios::binary);
	if (out.is_open()) {
		out.write((const char *)raw_data.data(), raw_data.size());
		out.close();
	} else {
		LibvideoLog() << "Could not open " + filename + "\n";
	}
}
#endif

void raw_yuv_to_frame(struct AVFrame *frame, const std::vector<uint8_t> &yuv_data) {
	const size_t upos = frame->width * frame->height;
	const size_t vpos = upos + upos / 4;

	const unsigned char *src_data[] = {yuv_data.data(), yuv_data.data() + upos, yuv_data.data() + vpos};

	const int src_linesize[4] = {frame->width, frame->width / 2, frame->width / 2, 0};

	av_image_copy(frame->data, frame->linesize, src_data, src_linesize, AV_PIX_FMT_YUV420P, frame->width, frame->height);
}

std::vector<uint8_t> get_extradata_by_enc_params(const EncoderParams &params) {
	auto codec = avcodec_find_encoder(AV_CODEC_ID_H264);

	auto codec_ctx = avcodec_alloc_context3(codec);

	codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	codec_ctx->bit_rate = params.bit_rate;
	codec_ctx->time_base = {1, params.framerate + 1};
	codec_ctx->framerate = {params.framerate + 1, 1};

	codec_ctx->gop_size = 0;

	codec_ctx->width = params.width;
	codec_ctx->height = params.height;

	codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

	if (codec->id == AV_CODEC_ID_H264)
		av_opt_set(codec_ctx->priv_data, "preset", "faster", 0);

	avcodec_open2(codec_ctx, codec, nullptr);

	std::vector<uint8_t> extradata(codec_ctx->extradata, codec_ctx->extradata + codec_ctx->extradata_size);

	avcodec_free_context(&codec_ctx);

	return extradata;
}

const char *libvideo_h264preset_to_string(enum LibvideoH264Preset preset) {
	switch (preset) {
	case LibvideoH264Preset::Ultrafast:
		return "ultrafast";

	case LibvideoH264Preset::Superfast:
		return "superfast";

	case LibvideoH264Preset::Veryfast:
		return "veryfast";

	case LibvideoH264Preset::Faster:
		return "faster";

	case LibvideoH264Preset::Fast:
		return "fast";

	case LibvideoH264Preset::Medium:
		return "medium";

	case LibvideoH264Preset::Slow:
		return "slow";

	case LibvideoH264Preset::Slower:
		return "slower";

	case LibvideoH264Preset::Veryslow:
		return "veryslow";

	default:
		return "faster";
	}
}
} // namespace Utilities

namespace HelpFuncs {
std::vector<uint8_t> rgb_to_yuv(const std::vector<uint8_t> &rgba_data, const int width, const int height) {
	std::vector<uint8_t> yuv_data(width * height * 3 / 2, 0);
	size_t upos = height * width;
	size_t vpos = upos + upos / 4;
	size_t i = 0;

	for (int line = 0; line < height; ++line) {
		if (!(line % 2)) {
			for (int x = 0; x < width; x += 2) {
				uint8_t r = rgba_data[4 * i];
				uint8_t g = rgba_data[4 * i + 1];
				uint8_t b = rgba_data[4 * i + 2];

				yuv_data[i++] = ((unsigned int)(66 * r + 129 * g + 25 * b) >> 8u) + 16u;

				yuv_data[vpos++] = ((unsigned int)(-38 * r + -74 * g + 112 * b) >> 8u) + 128u;
				yuv_data[upos++] = ((unsigned int)(112 * r + -94 * g + -18 * b) >> 8u) + 128u;

				r = rgba_data[4 * i];
				g = rgba_data[4 * i + 1];
				b = rgba_data[4 * i + 2];

				yuv_data[i++] = ((unsigned int)(66 * r + 129 * g + 25 * b) >> 8u) + 16u;
			}
		} else {
			for (int x = 0; x < width; x += 1) {
				uint8_t r = rgba_data[4 * i];
				uint8_t g = rgba_data[4 * i + 1];
				uint8_t b = rgba_data[4 * i + 2];

				yuv_data[i++] = ((unsigned int)(66 * r + 129 * g + 25 * b) >> 8u) + 16u;
			}
		}
	}

	return yuv_data;
}

std::vector<uint8_t> scale_yuv(const std::vector<uint8_t> &yuv_data, int width, int height, float scale_coef) {
	const int scaled_width = (int)(width * scale_coef);
	const int scaled_height = (int)(height * scale_coef);
	const int scale_flag = ((scale_coef >= 1.f) ? SWS_FAST_BILINEAR : SWS_BICUBIC);
	std::vector<uint8_t> scaled_yuv_data(scaled_width * scaled_height * 4, 0);
	SwsContext *sws_scaler_ctx = sws_getContext(width, height, AV_PIX_FMT_YUV420P, scaled_width, scaled_height,
												AV_PIX_FMT_YUV420P, scale_flag, nullptr, nullptr, nullptr);
	if (!sws_scaler_ctx) {
		LibvideoLog() << "Couldn't initialize sw scaler\n";
	}

	const size_t scaled_upos = scaled_width * scaled_height;
	const size_t scaled_vpos = scaled_upos + scaled_upos / 4;

	uint8_t *dest[3] = {scaled_yuv_data.data(), scaled_yuv_data.data() + scaled_upos, scaled_yuv_data.data() + scaled_vpos};
	int dest_linesize[3] = {scaled_width, scaled_width / 2, scaled_width / 2};

	const size_t upos = width * height;
	const size_t vpos = upos + upos / 4;

	uint8_t *data[3] = {(uint8_t *)yuv_data.data(), (uint8_t *)(yuv_data.data() + upos), (uint8_t *)(yuv_data.data() + vpos)};
	int linesize[3] = {width, width / 2, width / 2};

	sws_scale(sws_scaler_ctx, data, linesize, 0, height, dest, dest_linesize);

	sws_freeContext(sws_scaler_ctx);

	return scaled_yuv_data;
}
} // namespace HelpFuncs

} // namespace tomsksoft::libvideo
