extern "C" {
#include <libavcodec/avcodec.h>
}

#include <array>

#include "libvideo.h"

#include "GeneralUtilities.h"

#include "SWEncoder.h"
#include "SWDecoder.h"

#ifdef WIN32
#include "WinHWEncoder.h"
#include "WinHWDecoder.h"
#endif

#ifdef __ANDROID_API__
#if __ANDROID_API__ > 20
#include "AndroidHWDecoder.h"
#endif
#endif

namespace tomsksoft::libvideo {

enum EncoderTypes CheckAvailableEncoders() {
	const std::array<std::string, 3> hw_encoders = {"h264_amf", "h264_qsv", "h264_nvenc"};

	std::vector<std::string> avalible_hw_encoders;

	for (const auto &enc : hw_encoders) {
		void *i = 0;
		const AVCodec *cur_codec = av_codec_iterate(&i);
		while (cur_codec) {
			cur_codec = av_codec_iterate(&i);
			if (!cur_codec)
				continue;
			if (strcmp(enc.c_str(), cur_codec->name) == 0)
				avalible_hw_encoders.emplace_back(enc);
		}
	}

	if (!avalible_hw_encoders.empty()) {
		LibvideoLog() << "Available hardware encoders: ";
		for (const auto &hw_enc : avalible_hw_encoders)
			LibvideoLog() << hw_enc << " ";

		LibvideoLog() << "\n";
		LibvideoLog() << "Input name of hardware encoder\nor input 'software' for "
						 "software encoding: ";

		std::string encoder_name;
		std::cin >> encoder_name;

		if (encoder_name == "software")
			return EncoderTypes::Software;

		auto result = std::find(begin(avalible_hw_encoders), end(avalible_hw_encoders), "h264_amf");
		if (encoder_name == "h264_amf" && result != std::end(avalible_hw_encoders))
			return EncoderTypes::AMF;

		result = std::find(begin(avalible_hw_encoders), end(avalible_hw_encoders), "h264_qsv");
		if (encoder_name == "h264_qsv" && result != std::end(avalible_hw_encoders))
			return EncoderTypes::QSV;

		result = std::find(begin(avalible_hw_encoders), end(avalible_hw_encoders), "h264_nvenc");
		if (encoder_name == "h264_nvenc" && result != std::end(avalible_hw_encoders))
			return EncoderTypes::CUDA;
	}

	LibvideoLog() << "Can't find any HW encoder. Initialize software encoder\n";
	return EncoderTypes::Software;
}

enum DecoderTypes CheckAvailableDecoders() {
	std::vector<std::string> avalible_hw_decoders;

	enum AVHWDeviceType type = AV_HWDEVICE_TYPE_NONE;

	LibvideoLog() << "Available hardware decoding device types: ";
	while ((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE) {
		LibvideoLog() << av_hwdevice_get_type_name(type) << " ";
		avalible_hw_decoders.emplace_back(av_hwdevice_get_type_name(type));
	}

	LibvideoLog() << "\n";

	if (!avalible_hw_decoders.empty()) {
		LibvideoLog() << "Input name of device for decoding\nor input 'software' "
						 "for software decoding: ";
		std::string device_name;
		std::cin >> device_name;

		if (device_name == "software")
			return DecoderTypes::Software;

		for (const auto &hw_decoder : avalible_hw_decoders) {
			if (hw_decoder != device_name)
				continue;

			type = av_hwdevice_find_type_by_name(device_name.c_str());

			switch (type) {
#ifdef WIN32
			case AV_HWDEVICE_TYPE_CUDA:
				return DecoderTypes::CUDA;

			case AV_HWDEVICE_TYPE_DXVA2:
				return DecoderTypes::DX9;

			case AV_HWDEVICE_TYPE_D3D11VA:
				return DecoderTypes::DX11;

			case AV_HWDEVICE_TYPE_QSV:
				return DecoderTypes::QSV;

			case AV_HWDEVICE_TYPE_OPENCL:
				return DecoderTypes::OPENCL;
#endif

#ifdef __ANDROID_API__
#if __ANDROID_API__ > 20
			case AV_HWDEVICE_TYPE_MEDIACODEC:
				return DecoderTypes::MEDIACODEC;
#endif
#endif

			default:
				return DecoderTypes::Software;
			}
		}
	}

	LibvideoLog() << "Can't find any HW decoder. Initialize software decoder\n";
	return DecoderTypes::Software;
}

IEncoder *CreateEncoder(const EncoderParams &enc_params, enum EncoderTypes enc_type) {
	if (enc_type == EncoderTypes::Software)
		return new SWEncoder(enc_params);
#ifdef WIN32
	else
		return new WinHWEncoder(enc_params, enc_type);
#elif __linux__
	return new SWEncoder(enc_params);
#endif
}

#ifdef __ANDROID_API__
#if __ANDROID_API__ > 20
IDecoder *CreateDecoder(const DecoderParams &dec_params, enum DecoderTypes dec_type, void *surface)
#endif
#else
IDecoder *CreateDecoder(const DecoderParams &dec_params, enum DecoderTypes dec_type)
#endif
{
	if (dec_type == DecoderTypes::Software)
		return new SWDecoder(dec_params);
#ifdef WIN32
	else
		return new WinHWDecoder(dec_params, dec_type);
#elif __ANDROID_API__
#if __ANDROID_API__ > 20
	else
		return new AndroidHWDecoder(dec_params, surface);
#endif
#elif __linux__
	return new SWDecoder(dec_params);
#endif
}

} // namespace tomsksoft::libvideo
