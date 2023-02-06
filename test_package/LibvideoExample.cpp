#include <thread>

#include "libvideo.h"

#include "TestWindow.h"

using namespace tomsksoft::libvideo;

int main() {
	EncoderParams enc_params;
	enc_params.width = 1280;
	enc_params.height = 720;
	enc_params.bit_rate = enc_params.width * enc_params.height * 4 * 8; // 4 bytes per pixel, 8 bits per byte
	enc_params.framerate = 30;
	enc_params.preset = LibvideoH264Preset::Faster;

	DecoderParams dec_params;
	dec_params.width = enc_params.width / 2;
	dec_params.height = enc_params.height / 2;

	FilterParams filter_params;

	filter_params.color_space = ColorSpace::RGBA;
	filter_params.filter_type = FilterType::Scale;

	filter_params.crop_params.cropped_width = 640;
	filter_params.crop_params.cropped_height = 360;
	filter_params.crop_params.x_start = 0;
	filter_params.crop_params.y_start = 0;

	filter_params.scale_params.scale_coef = 0.5f;

	std::vector<uint8_t> smpte_rgba_data;
	Utilities::fill_by_raw_SMPTE_rgba_data(smpte_rgba_data, enc_params.width, enc_params.height);
	Utilities::dump_image(smpte_rgba_data, "smpte_rgba_data");

	IEncoder *newenc = CreateEncoder(enc_params, CheckAvailableEncoders());
	IDecoder *newdec = CreateDecoder(dec_params, DecoderTypes::Software);
	newenc->add_output([newdec](std::vector<uint8_t> &encoded_data) { newdec->decode(encoded_data); });

	newenc->add_filter(filter_params);

	std::vector<uint8_t> dec_output_storage;
	newdec->add_output([&dec_output_storage](std::vector<uint8_t> &decoded_data) { dec_output_storage = decoded_data; });

	Window input_window(dec_params.width, dec_params.height, 0, "Output");

	newdec->add_output(input_window.get_hwnd());

	bool runningFrames = true;
	input_window.stop_encode_decode = [&runningFrames]() { runningFrames = false; };

	auto deliver_to_window = [&]() {
		while (runningFrames) {
			auto rgba_data = smpte_rgba_data;

			newenc->encode_frame(rgba_data);
		}
	};

	std::thread displaying(deliver_to_window);

	input_window.wait_frames();

	displaying.join();

	return 0;
}