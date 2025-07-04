//! グラフィックスに関するモジュール

#pragma once

#include "../config.hpp"
#include "../error.hpp"

namespace graphics {

void initialize(const Config &config);

void createMesh(
	const char *id,
	const unsigned int vertexCount,
	const float *vertices,
	const unsigned int indexCount,
	const float *indices
);

void beginRender();

void endRender();

void terminate();

} // namespace graphics
