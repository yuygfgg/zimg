#include <algorithm>
#include "common/pixel.h"
#include "basic_filter.h"

namespace zimg {
namespace graph {

RGBExtendFilter::RGBExtendFilter(unsigned width, unsigned height, PixelType type) : m_attr{ width, height, type } {}

auto RGBExtendFilter::get_flags() const -> filter_flags
{
	filter_flags flags{};
	flags.color = true;
	flags.same_row = true;
	flags.in_place = true;
	return flags;
}

auto RGBExtendFilter::get_image_attributes() const -> image_attributes { return m_attr; }

void RGBExtendFilter::process(void *, const ImageBuffer<const void> src[], const ImageBuffer<void> dst[], void *, unsigned i, unsigned left, unsigned right) const
{
	const uint8_t *src_y = static_buffer_cast<const uint8_t>(src)[0][i];
	uint8_t *dst_u = static_buffer_cast<uint8_t>(dst)[1][i];
	uint8_t *dst_v = static_buffer_cast<uint8_t>(dst)[2][i];

	size_t left_byte = static_cast<size_t>(left) * pixel_size(m_attr.type);
	size_t right_byte = static_cast<size_t>(right) * pixel_size(m_attr.type);

	std::copy(src_y + left_byte, src_y + right_byte, dst_u + left_byte);
	std::copy(src_y + left_byte, src_y + right_byte, dst_v + left_byte);
}


ValueInitializeFilter::ValueInitializeFilter(unsigned width, unsigned height, PixelType type, value_type val) :
	m_attr{ width, height, type },
	m_value(val)
{}

void ValueInitializeFilter::fill_b(void *ptr, size_t n) const
{
	std::fill_n(static_cast<uint8_t *>(ptr), n, m_value.b);
}

void ValueInitializeFilter::fill_w(void *ptr, size_t n) const
{
	std::fill_n(static_cast<uint16_t *>(ptr), n, m_value.w);
}

void ValueInitializeFilter::fill_f(void *ptr, size_t n) const
{
	std::fill_n(static_cast<float *>(ptr), n, m_value.f);
}

auto ValueInitializeFilter::get_flags() const -> filter_flags
{
	filter_flags flags{};
	flags.same_row = true;
	flags.in_place = true;
	return flags;
}

auto ValueInitializeFilter::get_image_attributes() const -> image_attributes { return m_attr; }

void ValueInitializeFilter::process(void *, const ImageBuffer<const void> *, const ImageBuffer<void> *dst, void *, unsigned i, unsigned left, unsigned right) const
{
	uint8_t *dst_p = static_buffer_cast<uint8_t>(*dst)[i] + static_cast<size_t>(left) * pixel_size(m_attr.type);
	switch (m_attr.type) {
	case PixelType::BYTE:
		fill_b(dst_p, right - left);
		break;
	case PixelType::WORD:
	case PixelType::HALF:
		fill_w(dst_p, right - left);
		break;
	case PixelType::FLOAT:
		fill_f(dst_p, right - left);
		break;
	}
}

} // namespace graph
} // namespace zimg
