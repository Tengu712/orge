#pragma once

#include "../config.hpp"
#include "../error.hpp"

#include <vulkan/vulkan.hpp>

namespace graphics::pipeline {

void initialize(const config::Config &config, const vk::Device &device, const vk::RenderPass &renderPass);

/// パイプラインをバインドする関数
///
/// パイプラインはunordered_mapで管理されているため、
/// 競合するパイプラインがある場合、最終的にどのパイプラインがバインドされるかわからない。
///
/// 初期化された後に呼ばれることを期待する。
void bind(const vk::CommandBuffer &commandBuffer, uint32_t pipelineCount, const char *const *pipelines);

void bindDescriptorSets(
	const vk::CommandBuffer &commandBuffer,
	const char *id,
	uint32_t count,
	uint32_t const *indices
);

void updateBufferDescriptor(
	const vk::Device &device,
	const char *bufferId,
	const char *pipelineId,
	uint32_t set,
	uint32_t index,
	uint32_t binding
);

void terminate(const vk::Device &device);

} // namespace graphics::pipeline
