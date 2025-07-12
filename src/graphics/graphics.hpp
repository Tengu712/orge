#pragma once

#include "../config.hpp"

namespace graphics {

void initialize(const config::Config &config);

void createMesh(
	const char *id,
	const uint32_t vertexCount,
	const float *vertices,
	const uint32_t indexCount,
	const uint32_t *indices
);

void terminate();

} // namespace graphics
