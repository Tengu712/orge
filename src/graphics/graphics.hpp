//! グラフィックスに関するモジュール

#pragma once

#include "../config.hpp"
#include "../error.hpp"

namespace graphics {

void initialize(const Config &config);

void createMesh(
	const char *id,
	const uint32_t vertexCount,
	const float *vertices,
	const uint32_t indexCount,
	const uint32_t *indices
);

void beginRender();

void endRender();

void terminate();

} // namespace graphics
