#ifndef LIBVIDEO_CROPFILTER_H
#define LIBVIDEO_CROPFILTER_H

#include "IFilter.h"

namespace tomsksoft::libvideo {

struct CropFilter final : public IFilter {
  public:
	CropFilter(int initial_width, int cropped_width, int cropped_height, int x_start_pos, int y_start_pos);
	~CropFilter() override = default;

	std::vector<uint8_t> filtering_rgba(const std::vector<uint8_t> &initial_data) override;
	struct AVFrame *filtering_yuv(struct AVFrame *frame) override;

	int get_width() const { return m_cropped_width; }
	int get_height() const { return m_cropped_height; }

  private:
	// RGBA
	unsigned int m_start_offset_rgba = 0;
	int m_new_line_offset_rgba = 0;

	std::vector<uint8_t> m_cropped_data;

	// YUV
	unsigned int m_start_offset_y = 0;
	int m_new_line_offset_y = 0;

	unsigned int m_start_offset_u_v = 0;
	int m_new_line_offset_u_v = 0;

	struct AVFrame *cropped_frame = nullptr;

	// General
	int m_cropped_width = 0;
	int m_cropped_height = 0;
	int m_x_start_pos = 0;
	int m_y_start_pos = 0;
};

} // namespace tomsksoft::libvideo

#endif // LIBVIDEO_CROPFILTER_H
