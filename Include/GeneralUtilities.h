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

#ifndef LIBVIDEO_UTILS_H
#define LIBVIDEO_UTILS_H

#include <string>
#include <vector>

#include "EncoderParams.h"

namespace tomsksoft::libvideo {

/**
 * Utilities to help test and use the library.
 */

namespace Utilities {
/**
 * Fill container with RGBA test image.
 *
 * @param raw_data	std::vector in which raw RGBA test image will be stored.
 * @param width		Width of image.
 * @param height	Width of image.
 */
void fill_by_raw_rgba_data(std::vector<uint8_t> &raw_data, const int width, const int height);

/**
 * Fill container with SMPTE image.
 *
 * @param raw_data	std::vector in which raw RGBA SMPTE image will be
 * stored.
 * @param width		Width of image.
 * @param height	Height of image.
 */
void fill_by_raw_SMPTE_rgba_data(std::vector<uint8_t> &raw_data, const int width, const int height);

/**
 * Convert YUV raw data stored in std::vector to AVFrame.
 *
 * @param frame		AVFrame in which YUV data will be stored.
 * @param yuv_data	std::vector with stored raw YUV data.
 */
void raw_yuv_to_frame(struct AVFrame *frame, const std::vector<uint8_t> &yuv_data);

/**
 * Dump raw image data stored in container to file.
 *
 * @param raw_data	std::vector with stored raw RGBA data.
 * @param filename	filename in which image will be stored.
 */
#ifdef WIN32
void dump_image(const std::vector<uint8_t> &raw_data, const std::string &filename);
#endif

/**
 * Stored in std::vector SPS and PPS data with given EncoderParams for "faster" preset and GoP size 0.
 *
 * @param params	Encoder initialize parameters.
 *
 * @return          SPS and PPS data stored in std::vector.
 */
[[maybe_unused]] std::vector<uint8_t> get_extradata_by_enc_params(const EncoderParams &params);

/**
 * Transform libvideo enum preset to string.
 *
 * @param preset	libvideo H.264 encoder preset.
 *
 * @return          FFmpeg H.264 encoder preset.
 */
const char *libvideo_h264preset_to_string(enum LibvideoH264Preset preset);
} // namespace Utilities

/**
 * Help functions which are not used, but may be useful in the future.
 */

namespace HelpFuncs {
/**
 * Convert raw RGB image to YUV.
 *
 * @param rgba_data	std::vector with stored raw RGBA data.
 * @param width		Width of converted image.
 * @param height	Height of converted image.
 * @return			std::vector with stored raw YUV data.
 */
[[maybe_unused]] std::vector<uint8_t> rgb_to_yuv(const std::vector<uint8_t> &rgba_data, const int width, const int height);

/**
 * Dump raw image data stored in container to file.
 *
 * @param yuv_data		std::vector with stored raw YUV data.
 * @param width			Width of scaled image.
 * @param height		Height of scaled image.
 * @param scale_coef	Coefficient by which the image will be scaled.
 * @return				std::vector with stored scaled raw YUV
 * data.
 */
[[maybe_unused]] std::vector<uint8_t> scale_yuv(const std::vector<uint8_t> &yuv_data, int width, int height, float scale_coef);
} // namespace HelpFuncs

} // namespace tomsksoft::libvideo

#endif // LIBVIDEO_UTILS_H