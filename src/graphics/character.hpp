#pragma once

#include <stdint.h>

namespace graphics {

struct Character {
	const uint32_t x;
	const uint32_t y;
	const float u;
	const float v;
	const float w;
	const float h;
	const float ox;
	const float oy;
	const float advance;

	Character(uint32_t x, uint32_t y, float u, float v, float w, float h, float ox, float oy, float advance):
		x(x),
		y(y),
		u(u),
		v(v),
		w(w),
		h(h),
		ox(ox),
		oy(oy),
		advance(advance)
	{}
};

} // namespace graphics
