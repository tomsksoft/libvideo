#ifndef LIBVIDEO_RGBA_TO_YUV_CONVERTOR_H
#define LIBVIDEO_RGBA_TO_YUV_CONVERTOR_H

#include <vector>

#include "GeneralUtilities.h"

namespace tomsksoft::libvideo {

struct RGBAtoYUVConvertor {
  public:
	RGBAtoYUVConvertor(int width, int height);
	~RGBAtoYUVConvertor();

	std::vector<uint8_t> rgba_to_yuv(const std::vector<uint8_t> &rgba_data);

  private:
	int m_height = 0;

	struct SwsContext *m_rgba_to_yuv_ctx = nullptr;
	std::vector<uint8_t> m_yuv_data;
	size_t m_upos = 0;
	size_t m_vpos = 0;

	uint8_t *m_dest_data[3];
	int m_dest_linesize[3];
	int m_cur_linesize[1];
};

} // namespace tomsksoft::libvideo

#endif // LIBVIDEO_RGBA_TO_YUV_CONVERTOR_H