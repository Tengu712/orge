#pragma once

#include "../config.hpp"

namespace graphics {

void initialize(const config::Config &config);

void updateBufferDescriptor(
	const char *bufferId,
	const char *pipelineId,
	uint32_t set,
	uint32_t index,
	uint32_t binding
);

void createMesh(
	const char *id,
	const uint32_t vertexCount,
	const float *vertices,
	const uint32_t indexCount,
	const uint32_t *indices
);

void terminate();

} // namespace graphics
