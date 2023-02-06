/**
 * Encode and decode unit-test
 */

#include <gtest/gtest.h>

#include "libvideo.h"

using namespace tomsksoft::libvideo;

class TestLibVideo : public ::testing::Test {
  protected:
	EncoderParams enc_params;
	DecoderParams dec_params;

	IEncoder *sut_enc = nullptr;
	IDecoder *sut_dec = nullptr;

	std::vector<uint8_t> smpte_rgba_data;
	std::vector<uint8_t> dec_output_storage;

	void SetUp() {
		enc_params.width = 320;
		enc_params.height = 240;
		enc_params.bit_rate = enc_params.width * enc_params.height * 4 * 8; // 4 bytes per pixel, 8 bits per byte

		dec_params.width = enc_params.width;
		dec_params.height = enc_params.height;

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

TEST_F(TestLibVideo, is_decoded_and_initial_frame_has_same_size) {
	// Arrange
	// in fixture

	// Act
	sut_enc->encode_frame(smpte_rgba_data);

	Utilities::dump_image(dec_output_storage, "is_decoded_and_initial_frame_has_same_size");

	// Assert
	EXPECT_EQ(smpte_rgba_data.size(), dec_output_storage.size());
}