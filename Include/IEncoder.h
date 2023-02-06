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

#ifndef LIBVIDEO_IENCODER_H
#define LIBVIDEO_IENCODER_H

// #include <Windows.h>
#include <functional>

namespace tomsksoft::libvideo {

/**
 * Encoder for raw video stream.
 */

class IEncoder {
  public:
	/**
	 * Add a callback which will be called every "encode_frame()" call
	 * and send encoded data as std::vector<uint8_t>& type to this callback.
	 *
	 * @param out_callback	Created by user callback.
	 */
	virtual void add_output(std::function<void(std::vector<uint8_t> &)> &&out_callback) = 0;

	/**
	 * Add crop/scale filter for decoded frame.
	 *
	 * @param f_params		Filtering parameters.
	 */
	virtual void add_filter(const struct FilterParams &f_params) = 0;

	/**
	 * Encode raw RGBA video stream.
	 *
	 * @param video_frame		Raw video frame stored in std::vector.
	 */
	virtual void encode_frame(std::vector<uint8_t> &video_frame) = 0;

	/**
	 * On/off logs: logs: frame size, encoded size. Logs appear every
	 * "encode_frame()" call.
	 *
	 * @param enable_logs		true for enable logs, false for disable logs.
	 */
	virtual void set_logs_active(bool enable_logs) = 0;

	virtual ~IEncoder() = default;
};

} // namespace tomsksoft::libvideo

#endif // LIBVIDEO_IENCODER_H