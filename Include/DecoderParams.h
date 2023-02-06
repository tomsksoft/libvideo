/*
 * libvideo
 * Copyright (C) 2022 Tomsksoft
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef LIBVIDEO_DECODER_PARAMS_H
#define LIBVIDEO_DECODER_PARAMS_H

#include <vector>

namespace tomsksoft::libvideo {

/**
 * Possible types of created Decoder.
 */

enum class DecoderTypes : uint8_t {
	Software = 0,
#ifdef WIN32
	CUDA = 1,
	DX9 = 2,
	DX11 = 3,
	QSV = 4,
	OPENCL = 5
#endif

#ifdef __ANDROID_API__
#if __ANDROID_API__ > 20
		MEDIACODEC = 1
#endif
#endif
};

/**
 * Parameters for creating a Decoder.
 */

struct DecoderParams {
	/**
	 * Width of decoded frame.
	 */
	int width = 0;

	/**
	 * Height of decoded frame.
	 */
	int height = 0;

#ifdef __ANDROID_API__
#if __ANDROID_API__ > 20
	/**
	 * H.264 SPS and PPS data. If user have array of SPS and PPS data he can load extradata to
	 * this storage, if have't extradata automatically filled by stream.
	 * Only for Android MediaCodec decoder.
	 */
	std::vector<uint8_t> extradata;
#endif
#endif
};

} // namespace tomsksoft::libvideo

#endif // LIBVIDEO_DECODER_PARAMS_H