#ifndef LIBVIDEO_SCALEFILTER_H
#define LIBVIDEO_SCALEFILTER_H

#include <memory>

#include "IFilter.h"

namespace tomsksoft::libvideo {

struct ScaleFilter final : public IFilter {
  public:
	ScaleFilter(int initial_width, int initial_height, float scale_coef);
	~ScaleFilter() override;

	std::vector<uint8_t> filtering_rgba(const std::vector<uint8_t> &initial_data) override;
	struct AVFrame *filtering_yuv(struct AVFrame *frame) override;

	float get_scale_coef() const { return m_scale_coef; }

  private:
	// RGBA
	struct SwsContext *m_scale_rgba_ctx = nullptr;
	std::vector<uint8_t> m_scaled_rgba_data;

	int m_dest_linesize[1];
	int m_src_linesize[1];

	// YUV
	struct SwsContext *m_scale_yuv_ctx = nullptr;
	struct AVFrame *m_scaled_frame = nullptr;

	// General
	float m_scale_coef = 1.f;

	int m_initial_height = 0;

	int m_scaled_width = 0;
	int m_scaled_height = 0;
};

} // namespace tomsksoft::libvideo

#endif // LIBVIDEO_SCALEFILTER_H
