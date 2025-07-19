#pragma once

#include "../../../config/config.hpp"

#include <vulkan/vulkan.hpp>

namespace graphics::rendering::pipeline {

void terminate(const vk::Device &device);

void initialize(
	const config::Config &config,
	const vk::Device &device,
	const vk::CommandPool &commandPool,
	const vk::RenderPass &renderPass
);

void bind(const vk::Device &device, const vk::CommandBuffer &commandBuffer, uint32_t index, const char *id);

void bindDescriptorSets(const vk::CommandBuffer &commandBuffer, const char *id, uint32_t const *indices);

void updateBufferDescriptor(
	const vk::Device &device,
	const char *bufferId,
	const char *pipelineId,
	uint32_t set,
	uint32_t index,
	uint32_t binding
);

void updateImageDescriptor(
	const vk::Device &device,
	const char *imageId,
	const char *pipelineId,
	uint32_t set,
	uint32_t index,
	uint32_t binding
);

} // namespace graphics::rendering::pipeline
