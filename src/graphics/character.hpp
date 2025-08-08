#pragma once

#include <stdint.h>

namespace graphics {

struct Character {
	/// テクスチャアトラス上のマスの左上原点の左座標
	uint32_t offsetX;
	/// テクスチャアトラス上のマスの左上原点の上座標
	uint32_t offsetY;

	Character(uint32_t offsetX, uint32_t offsetY): offsetX(offsetX), offsetY(offsetY) {}
};

} // namespace graphics
