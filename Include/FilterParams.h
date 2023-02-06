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

#ifndef LIBVIDEO_FILTER_PARAMS_H
#define LIBVIDEO_FILTER_PARAMS_H

#include <vector>

namespace tomsksoft::libvideo {

/**
 * Colorspace in which the filter will be used.
 */

enum class ColorSpace {
	RGBA = 0,
	YUV = 1
};

/**
 * Type of filter. Crop for cropping frame. Scale for scaling frame.
 */

enum class FilterType {
	Crop = 0,
	Scale = 1
};

/**
 * Initialize filters for Encoder and Decoder.
 */

struct FilterParams {
	/**
	 * Parameters for scale filter.
	 */

	struct ScaleParams {
		/**
		 * Coefficient by which the image will be scaled.
		 */
		float scale_coef = 1.f;
	};
	ScaleParams scale_params;

	/**
	 * Parameters for crop filter.
	 */

	struct CropParams {
		/**
		 * Cut width from original image.
		 */
		int cropped_width = 0;

		/**
		 * Cut height from original image.
		 */
		int cropped_height = 0;

		/**
		 * X coordinate of the top left cut rectangle.
		 */
		int x_start = 0;

		/**
		 * Y coordinate of the top left cut rectangle.
		 */
		int y_start = 0;
	};
	CropParams crop_params;

	enum ColorSpace color_space = ColorSpace::RGBA;

	enum FilterType filter_type = FilterType::Crop;
};

} // namespace tomsksoft::libvideo

#endif // LIBVIDEO_FILTER_PARAMS_H