/**
 * Set of unit-tests library filtres
 */

#include <gtest/gtest.h>

#include "libvideo.h"

using namespace tomsksoft::libvideo;

class TestLibVideoFilter : public ::testing::Test {
  protected:
	EncoderParams enc_params;
	DecoderParams dec_params;
	FilterParams filter_params;

	IEncoder *sut_enc = nullptr;
	IDecoder *sut_dec = nullptr;

	std::vector<uint8_t> smpte_rgba_data;
	std::vector<uint8_t> dec_output_storage;

	void SetUp() {
		enc_params.width = 1280;
		enc_params.height = 720;
		enc_params.bit_rate = enc_params.width * enc_params.height * 4 * 8; // 4 bytes per pixel, 8 bits per byte

		dec_params.width = enc_params.width;
		dec_params.height = enc_params.height;

		filter_params.scale_params.scale_coef = 2.f;

		filter_params.crop_params.cropped_width = 640;
		filter_params.crop_params.cropped_height = 360;
		filter_params.crop_params.x_start = 0;
		filter_params.crop_params.y_start = 0;

		Utilities::fill_by_raw_SMPTE_rgba_data(smpte_rgba_data, enc_params.width, enc_params.height);

		sut_enc = CreateEncoder(enc_params, EncoderTypes::Software);
		sut_dec = CreateDecoder(dec_params, DecoderTypes::Software);

		sut_enc->add_output([this](std::vector<uint8_t> &encoded_data) { sut_dec->decode(encoded_data); });
		sut_dec->add_output([this](std::vector<uint8_t> &decoded_data) { dec_output_storage = decoded_data; });
	}
	void TearDown() {
		delete sut_enc;
		delete sut_dec;
	}
};

TEST_F(TestLibVideoFilter, is_scaled_frame_has_scaled_size_enc_yuv) {
	// Arrange
	filter_params.color_space = ColorSpace::YUV;
	filter_params.filter_type = FilterType::Scale;
	sut_enc->add_filter(filter_params);

	std::vector<uint8_t> rgba_data = smpte_rgba_data;

	// Act
	sut_enc->encode_frame(rgba_data);

	Utilities::dump_image(dec_output_storage, "is_scaled_frame_has_scaled_size_enc_yuv");

	// Assert
	int asserted_size =
		(int)(smpte_rgba_data.size() * filter_params.scale_params.scale_coef * filter_params.scale_params.scale_coef);
	EXPECT_EQ(asserted_size, dec_output_storage.size());
}

TEST_F(TestLibVideoFilter, is_cropped_frame_has_cropped_size_enc_yuv) {
	// Arrange
	filter_params.color_space = ColorSpace::YUV;
	filter_params.filter_type = FilterType::Crop;
	sut_enc->add_filter(filter_params);

	std::vector<uint8_t> rgba_data = smpte_rgba_data;

	// Act
	sut_enc->encode_frame(rgba_data);

	Utilities::dump_image(dec_output_storage, "is_cropped_frame_has_cropped_size_enc_yuv");

	// Assert
	int crop_coef_w = enc_params.width / filter_params.crop_params.cropped_width;
	int crop_coef_h = enc_params.height / filter_params.crop_params.cropped_height;
	int asserted_size = (int)(smpte_rgba_data.size() / (crop_coef_w * crop_coef_h));
	EXPECT_EQ(asserted_size, dec_output_storage.size());
}

TEST_F(TestLibVideoFilter, is_scaled_frame_has_scaled_size_enc_rgba) {
	// Arrange
	filter_params.color_space = ColorSpace::RGBA;
	filter_params.filter_type = FilterType::Scale;
	sut_enc->add_filter(filter_params);

	std::vector<uint8_t> rgba_data = smpte_rgba_data;

	// Act
	sut_enc->encode_frame(rgba_data);

	Utilities::dump_image(dec_output_storage, "is_scaled_frame_has_scaled_size_enc_rgba");

	// Assert
	int asserted_size =
		(int)(smpte_rgba_data.size() * filter_params.scale_params.scale_coef * filter_params.scale_params.scale_coef);
	EXPECT_EQ(asserted_size, dec_output_storage.size());
}

TEST_F(TestLibVideoFilter, is_cropped_frame_has_cropped_size_enc_rgba) {
	// Arrange
	filter_params.color_space = ColorSpace::RGBA;
	filter_params.filter_type = FilterType::Crop;
	sut_enc->add_filter(filter_params);

	std::vector<uint8_t> rgba_data = smpte_rgba_data;

	// Act
	sut_enc->encode_frame(rgba_data);

	Utilities::dump_image(dec_output_storage, "is_cropped_frame_has_cropped_size_enc_rgba");

	// Assert
	int crop_coef_w = enc_params.width / filter_params.crop_params.cropped_width;
	int crop_coef_h = enc_params.height / filter_params.crop_params.cropped_height;
	int asserted_size = (int)(smpte_rgba_data.size() / (crop_coef_w * crop_coef_h));
	EXPECT_EQ(asserted_size, dec_output_storage.size());
}

TEST_F(TestLibVideoFilter, is_scaled_frame_has_scaled_size_dec_yuv) {
	// Arrange
	filter_params.color_space = ColorSpace::YUV;
	filter_params.filter_type = FilterType::Scale;
	sut_dec->add_filter(filter_params);

	std::vector<uint8_t> rgba_data = smpte_rgba_data;

	// Act
	sut_enc->encode_frame(rgba_data);

	Utilities::dump_image(dec_output_storage, "is_scaled_frame_has_scaled_size_dec_yuv");

	// Assert
	int asserted_size =
		(int)(smpte_rgba_data.size() * filter_params.scale_params.scale_coef * filter_params.scale_params.scale_coef);
	EXPECT_EQ(asserted_size, dec_output_storage.size());
}

TEST_F(TestLibVideoFilter, is_cropped_frame_has_cropped_size_dec_yuv) {
	// Arrange
	filter_params.color_space = ColorSpace::YUV;
	filter_params.filter_type = FilterType::Crop;
	sut_dec->add_filter(filter_params);

	std::vector<uint8_t> rgba_data = smpte_rgba_data;

	// Act
	sut_enc->encode_frame(rgba_data);

	Utilities::dump_image(dec_output_storage, "is_cropped_frame_has_cropped_size_dec_yuv");

	// Assert
	int crop_coef_w = enc_params.width / filter_params.crop_params.cropped_width;
	int crop_coef_h = enc_params.height / filter_params.crop_params.cropped_height;
	int asserted_size = (int)(smpte_rgba_data.size() / (crop_coef_w * crop_coef_h));
	EXPECT_EQ(asserted_size, dec_output_storage.size());
}

TEST_F(TestLibVideoFilter, is_scaled_frame_has_scaled_size_dec_rgba) {
	// Arrange
	filter_params.color_space = ColorSpace::RGBA;
	filter_params.filter_type = FilterType::Scale;
	sut_dec->add_filter(filter_params);

	std::vector<uint8_t> rgba_data = smpte_rgba_data;

	// Act
	sut_enc->encode_frame(rgba_data);

	Utilities::dump_image(dec_output_storage, "is_scaled_frame_has_scaled_size_dec_rgba");

	// Assert
	int asserted_size =
		(int)(smpte_rgba_data.size() * filter_params.scale_params.scale_coef * filter_params.scale_params.scale_coef);
	EXPECT_EQ(asserted_size, dec_output_storage.size());
}

TEST_F(TestLibVideoFilter, is_cropped_frame_has_cropped_size_dec_rgba) {
	// Arrange
	filter_params.color_space = ColorSpace::RGBA;
	filter_params.filter_type = FilterType::Crop;
	sut_dec->add_filter(filter_params);

	std::vector<uint8_t> rgba_data = smpte_rgba_data;

	// Act
	sut_enc->encode_frame(rgba_data);

	Utilities::dump_image(dec_output_storage, "is_cropped_frame_has_cropped_size_dec_rgba");

	// Assert
	int crop_coef_w = enc_params.width / filter_params.crop_params.cropped_width;
	int crop_coef_h = enc_params.height / filter_params.crop_params.cropped_height;
	int asserted_size = (int)(smpte_rgba_data.size() / (crop_coef_w * crop_coef_h));
	EXPECT_EQ(asserted_size, dec_output_storage.size());
}