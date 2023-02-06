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

#ifndef LIBVIDEO_IDECODER_H
#define LIBVIDEO_IDECODER_H

#ifdef WIN32
#include <Windows.h>
#endif

#include <functional>

namespace tomsksoft::libvideo {

/**
 * Decoder for H.264 video stream.
 */

class IDecoder {
  public:
#ifdef WIN32
	/**
	 * Add a window in which the decoded frame will be drawn every "decode()"
	 * call.
	 *
	 * @param hwnd			HWND for created by user window.
	 */
	virtual void add_output(HWND hwnd) = 0;
#endif

	/**
	 * Add a callback which will be called every "decode()" call
	 * and send decoded data as std::vector<uint8_t>& type to this callback.
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
	 * Decode H.264 video stream data.
	 *
	 * @param encoded_data	H.264 data stored in std::vector
	 */
	virtual void decode(std::vector<uint8_t> &encoded_data) = 0;

	/**
	 * On/off logs: resolution, bitrate, framerate. Logs appear every "decode()"
	 * call.
	 *
	 * @param enable_logs		true for enable logs, false for disable logs.
	 */
	virtual void set_logs_active(bool enable_logs) = 0;

	virtual ~IDecoder() = default;
};

} // namespace tomsksoft::libvideo

#endif // LIBVIDEO_IDECODER_H
