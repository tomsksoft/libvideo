#ifndef LIBVIDEO_IFILTER_H
#define LIBVIDEO_IFILTER_H

#include "GeneralUtilities.h"

namespace tomsksoft::libvideo {

struct IFilter {
  public:
	virtual ~IFilter() = default;
	virtual std::vector<uint8_t> filtering_rgba(const std::vector<uint8_t> &initial_data) = 0;
	virtual struct AVFrame *filtering_yuv(struct AVFrame *frame) = 0;
};

} // namespace tomsksoft::libvideo

#endif // LIBVIDEO_IFILTER_H
