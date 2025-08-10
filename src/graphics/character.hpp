#pragma once

#include <stdint.h>

namespace graphics {

struct Character {
	const uint32_t x;
	const uint32_t y;
	const float w;

	const float ox;
	const float oy;
	const float advance;

	const float u;
	const float v;

	Character(
		uint32_t x,
		uint32_t y,
		float w,
		float ox,
		float oy,
		float advance,
		float u,
		float v
	):
		x(x),
		y(y),
		w(w),
		ox(ox),
		oy(oy),
		advance(advance),
		u(u),
		v(v)
	{}
};

struct ScaledCharacter {
	const float w;

	const float ox;
	const float oy;
	const float advance;

	const float u;
	const float v;

	ScaledCharacter(float w, float ox, float oy, float advance, float u, float v):
		w(w), ox(ox), oy(oy), advance(advance), u(u), v(v)
	{}
};

} // namespace graphics
