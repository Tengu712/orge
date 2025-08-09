#pragma once

#include <stdint.h>

namespace graphics {

struct Character {
	const uint32_t x;
	const uint32_t y;

	const float ox;
	const float oy;
	const float advance;

	const float u;
	const float v;

	Character(
		uint32_t x,
		uint32_t y,
		float ox,
		float oy,
		float advance,
		float u,
		float v
	):
		x(x),
		y(y),
		ox(ox),
		oy(oy),
		advance(advance),
		u(u),
		v(v)
	{}
};

} // namespace graphics
