#ifndef LIBVIDEO_YUV_TO_RGBA_CONVERTOR_H
#define LIBVIDEO_YUV_TO_RGBA_CONVERTOR_H

#include <vector>

#include "GeneralUtilities.h"

namespace tomsksoft::libvideo {

struct YUVtoRGBAConvertor {
  public:
	YUVtoRGBAConvertor(int width, int height);
	~YUVtoRGBAConvertor();

	std::vector<uint8_t> yuv_to_rgba(const struct AVFrame *frame);

  private:
	struct SwsContext *m_yuv_to_rgba_ctx = nullptr;
	std::vector<uint8_t> m_rgba_decoded_data;

	int m_dest_linesize[1];
};

} // namespace tomsksoft::libvideo

#endif // LIBVIDEO_YUV_TO_RGBA_CONVERTOR_H