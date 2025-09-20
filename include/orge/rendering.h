#pragma once

#include <stdint.h>

struct OrgeBindMeshParam {
	const char *meshId;
};

struct OrgeBeginRenderPassParam {
	const char *renderPassId;
};

struct OrgeBindPipelineParam {
	const char *pipelineId;
	const uint32_t *indices;
};

struct OrgeDrawParam {
	uint32_t instanceCount;
	uint32_t instanceOffset;
};

struct OrgeDrawDirectlyParam {
	uint32_t vertexCount;
	uint32_t instanceCount;
	uint32_t instanceOffset;
};
