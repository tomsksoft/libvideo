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

#ifndef LIBVIDEO_ENCODER_PARAMS_H
#define LIBVIDEO_ENCODER_PARAMS_H

#include <vector>

namespace tomsksoft::libvideo {

/**
 * Possible types of created Encoder.
 */

enum class EncoderTypes : uint8_t {
	Software = 0,
	CUDA = 1,
	AMF = 2,
	QSV = 3
};

/**
 * Possible types of H.264 encoder preset.
 */

enum class LibvideoH264Preset : uint8_t {
	Ultrafast = 0,
	Superfast = 1,
	Veryfast = 2,
	Faster = 3,
	Fast = 4,
	Medium = 5,
	Slow = 6,
	Slower = 7,
	Veryslow = 8
};

/**
 * Parameters for creating a Decoder.
 */

struct EncoderParams {
	/**
	 * Width of encoded frame.
	 */
	int width = 0;

	/**
	 * Height of encoded frame.
	 */
	int height = 0;

	/**
	 * Bitrate of encoded video stream.
	 */
	int64_t bit_rate = 0;

	/**
	 * Framerate of encoded video stream.
	 */
	int framerate = 30;

	/**
	 * H.264 encoder preset.
	 */
	enum LibvideoH264Preset preset = LibvideoH264Preset::Faster;
};

} // namespace tomsksoft::libvideo

#endif // LIBVIDEO_ENCODER_PARAMS_H