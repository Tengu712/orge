#pragma once

#include <stdint.h>

struct OrgeCreateBufferParam {
	const char *id;
	uint64_t size;
	uint8_t isStorage;
	uint8_t isHostCoherent;
};

struct OrgeDestroyBufferParam {
	const char *id;
};

struct OrgeUpdateBufferParam {
	const char *id;
	const uint8_t *data;
};

struct OrgeCopyBufferToParam {
	const char *id;
	uint8_t *data;
};

struct OrgeUpdateBufferDescriptorParam {
	const char *renderPassId;
	const char *pipelineId;
	const char *id;
	uint32_t set;
	uint32_t index;
	uint32_t binding;
	uint32_t offset;
};

struct OrgeUpdateComputeBufferDescriptorParam {
	const char *pipelineId;
	const char *id;
	uint32_t set;
	uint32_t index;
	uint32_t binding;
	uint32_t offset;
};

struct OrgeLoadImageParam {
	const char *file;
};

struct OrgeDestroyImageParam {
	const char *file;
};

struct OrgeCreateStorageImageParam {
	const char *id;
	uint32_t width;
	uint32_t height;
	uint32_t format;
};

struct OrgeDestroyStorageImageParam {
	const char *id;
};

struct OrgeUpdateImageDescriptorParam {
	const char *renderPassId;
	const char *pipelineId;
	const char *id;
	uint32_t set;
	uint32_t index;
	uint32_t binding;
	uint32_t offset;
};

struct OrgeUpdateComputeImageDescriptorParam {
	const char *pipelineId;
	const char *id;
	uint32_t set;
	uint32_t index;
	uint32_t binding;
	uint32_t offset;
};

struct OrgeUpdateComputeStorageImageDescriptorParam {
	const char *pipelineId;
	const char *id;
	uint32_t set;
	uint32_t index;
	uint32_t binding;
	uint32_t offset;
};

struct OrgeCreateSamplerParam {
	const char *id;
	uint8_t linearMagFilter;
	uint8_t linearMinFilter;
	uint8_t repeat;
};

struct OrgeDestroySamplerParam {
	const char *id;
};

struct OrgeUpdateSamplerDescriptorParam {
	const char *renderPassId;
	const char *pipelineId;
	const char *id;
	uint32_t set;
	uint32_t index;
	uint32_t binding;
	uint32_t offset;
};

struct OrgeUpdateComputeSamplerDescriptorParam {
	const char *pipelineId;
	const char *id;
	uint32_t set;
	uint32_t index;
	uint32_t binding;
	uint32_t offset;
};

struct OrgeUpdateInputAttachmentDescriptorParam {
	const char *renderPassId;
	const char *pipelineId;
	const char *id;
	uint32_t set;
	uint32_t index;
	uint32_t binding;
	uint32_t offset;
};

struct OrgeLoadMeshParam {
	const char *id;
};

struct OrgeDestroyMeshParam {
	const char *id;
};
