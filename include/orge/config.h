#pragma once

#include <stdint.h>

struct OrgeFontConfig {
	const char *id;
	// .ttfファイル名
	const char *file;
	uint32_t charSize;
	uint32_t charAtlusCol;
	uint32_t charAtlusRow;
};

enum OrgeFormat {
	ORGE_FORMAT_RENDER_TARGET = 0,
	ORGE_FORMAT_DEPTH_BUFFER,
	ORGE_FORMAT_SHARE_COLOR_ATTACHMENT,
	ORGE_FORMAT_SIGNED_SHARE_COLOR_ATTACHMENT,
};

enum OrgeClearValueType {
	ORGE_CLEAR_VALUE_TYPE_COLOR = 0,
	ORGE_CLEAR_VALUE_TYPE_DEPTH,
};

struct OrgeAttachmentConfig {
	const char *id;
	OrgeFormat format;
	uint8_t discard;
	OrgeClearValueType clearValueType;
	// clearValueType == ORGE_CLEAR_VALUE_TYPE_DEPTHの場合、
	// 最初の要素だけ使用される
	float clearValue[4];
};

enum OrgeDescriptorType {
	ORGE_DESCRIPTOR_TYPE_TEXTURE = 0,
	ORGE_DESCRIPTOR_TYPE_SAMPLER,
	ORGE_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	ORGE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
	ORGE_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
};

enum OrgeShaderStages {
	ORGE_SHADER_STAGES_VERTEX = 0,
	ORGE_SHADER_STAGES_FRAGMENT,
	ORGE_SHADER_STAGES_VERTEX_AND_FRAGMENT,
};

struct OrgeDescriptorBindingConfig {
	OrgeDescriptorType type;
	uint32_t count;
	OrgeShaderStages stage;
};

struct OrgeDescriptorSetConfig {
	uint32_t count;
	uint32_t bindingCount;
	OrgeDescriptorBindingConfig *bindings;
};

struct OrgeGraphicsPipelineConfig {
	const char *id;
	const char *vertexShaderFile;
	const char *fragmentShaderFile;
	uint32_t descSetCount;
	OrgeDescriptorSetConfig *descSets;
	uint32_t vertexInputAttributeCount;
	uint32_t *vertexInputAttributes;
	uint8_t meshInShader;
	uint8_t culling;
	uint8_t depthTest;
	uint32_t colorBlendCount;
	uint8_t *colorBlends;
};

struct OrgeSubpassDepthConfig {
	const char *depth;
	uint8_t readOnly;
};

struct OrgeSubpassConfig {
	const char *id;
	uint32_t inputCount;
	const char **inputs;
	uint32_t outputCount;
	const char **outputs;
	OrgeSubpassDepthConfig *depth;
	uint32_t dependCount;
	const char **depends;
	uint32_t pipelineCount;
	const char **pipelines;
};

struct OrgeRenderPassConfig {
	const char *id;
	uint32_t subpassCount;
	OrgeSubpassConfig *subpasses;
};

enum OrgeComputeDescriptorType {
	ORGE_COMPUTE_DESCRIPTOR_TYPE_TEXTURE = 0,
	ORGE_COMPUTE_DESCRIPTOR_TYPE_SAMPLER,
	ORGE_COMPUTE_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	ORGE_COMPUTE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
	ORGE_COMPUTE_DESCRIPTOR_TYPE_STORAGE_IMAGE,
};

struct OrgeComputeDescriptorBindingConfig {
	OrgeComputeDescriptorType type;
	uint32_t count;
};

struct OrgeComputeDescriptorSetConfig {
	uint32_t count;
	uint32_t bindingCount;
	OrgeComputeDescriptorBindingConfig *bindings;
};

struct OrgeComputePipelineConfig {
	const char *id;
	const char *shaderFile;
	uint32_t descSetCount;
	OrgeComputeDescriptorSetConfig *descSets;
};

struct OrgeInitializeParam {
	const char *title;
	uint32_t width;
	uint32_t height;
	uint8_t fullscreen;
	uint8_t disableVsync;
	uint8_t altReturnToggleFullscreen;
	uint32_t audioChannelCount;
	uint32_t charCount;
	uint32_t assetCount;
	const char **assets;
	uint32_t fontCount;
	OrgeFontConfig *fonts;
	uint32_t attachmentCount;
	OrgeAttachmentConfig *attachments;
	uint32_t graphicsPipelineCount;
	OrgeGraphicsPipelineConfig *graphicsPipelines;
	uint32_t renderPassCount;
	OrgeRenderPassConfig *renderPasses;
	uint32_t computePipelineCount;
	OrgeComputePipelineConfig *computePipelines;
};
