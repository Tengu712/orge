#pragma once

#include <stdint.h>

struct OrgeBindComputePipelineParam {
	const char *pipelineId;
	const uint32_t *indices;
};

struct OrgeDispatchParam {
	uint32_t x;
	uint32_t y;
	uint32_t z;
};
