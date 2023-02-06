#ifndef LIBVIDEO_YUV_TO_NV12_CONVERTOR_H
#define LIBVIDEO_YUV_TO_NV12_CONVERTOR_H

#include <vector>

#include "GeneralUtilities.h"

namespace tomsksoft::libvideo {

struct YUVtoNV12Convertor {
  public:
	YUVtoNV12Convertor(int width, int height);
	~YUVtoNV12Convertor();

	void raw_yuv_to_nv12frame(struct AVFrame *dst_nv12_frame, const std::vector<uint8_t> &yuv_data);

  private:
	struct SwsContext *m_yuv_to_nv12_ctx = nullptr;
	size_t m_upos = 0;
	size_t m_vpos = 0;

	int m_src_linesize[3];
};

} // namespace tomsksoft::libvideo

#endif // LIBVIDEO_YUV_TO_NV12_CONVERTOR_H