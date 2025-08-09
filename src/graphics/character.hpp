#pragma once

#include <stdint.h>

namespace graphics {

struct Character {
	const uint32_t x;
	const uint32_t y;
	const float w;
	const float h;

	const float ox;
	const float oy;
	const float advance;

	const float u;
	const float v;
	const float ru;
	const float rv;

	Character(
		uint32_t x,
		uint32_t y,
		float w,
		float h,
		float ox,
		float oy,
		float advance,
		float u,
		float v,
		float ru,
		float rv
	):
		x(x),
		y(y),
		w(w),
		h(h),
		ox(ox),
		oy(oy),
		advance(advance),
		u(u),
		v(v),
		ru(ru),
		rv(rv)
	{}
};

} // namespace graphics
