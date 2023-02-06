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

#ifndef LIBVIDEO_INTERFACE_H
#define LIBVIDEO_INTERFACE_H

#include "EncoderParams.h"
#include "IEncoder.h"

#include "DecoderParams.h"
#include "IDecoder.h"

#include "FilterParams.h"

#include "GeneralUtilities.h"

namespace tomsksoft::libvideo {

/**
 * libvideo API.
 */

/**
 * Check available encoders for current system.
 *
 * @return	            Type of encoder selected among available encoders.
 */
enum EncoderTypes CheckAvailableEncoders();

/**
 * Check available decoders for current system.
 *
 * @return	            Type of decoder selected among available decoders.
 */
enum DecoderTypes CheckAvailableDecoders();

/**
 * Create Encoder.
 *
 * @param enc_params	EncoderParams struct.
 * @param enc_type	    EncoderType.
 * @return			    Pointer to created Encoder.
 */
IEncoder *CreateEncoder(const EncoderParams &enc_params, enum EncoderTypes enc_type);

/**
 * Create Decoder.
 *
 * @param dec_params	DecoderParams struct.
 * @param dec_type	    DecoderTypes.
 * @param surface       Only for Android. Reference to an android/view/Surface.
 *                      Set nullptr for Windows.
 * @return			    Pointer to created Decoder.
 */

#ifdef __ANDROID_API__
#if __ANDROID_API__ > 20
IDecoder *CreateDecoder(const DecoderParams &dec_params, enum DecoderTypes dec_type, void *surface = nullptr);
#endif
#else
#ifdef WIN32
IDecoder *CreateDecoder(const DecoderParams &dec_params, enum DecoderTypes dec_type);
#endif
#endif

} // namespace tomsksoft::libvideo

#endif // LIBVIDEO_INTERFACE_H