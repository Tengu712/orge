#pragma once

#include "../config.hpp"
#include "../error.hpp"

#include <vulkan/vulkan.hpp>

namespace graphics::rendering {

void initialize(const config::Config &config, const vk::Device &device, const vk::CommandPool &commandPool);

void beginRender(const vk::Device &device);

void bindDescriptorSets(
	const char *id,
	uint32_t count,
	uint32_t const *indices
);

void draw(
	uint32_t pipelineCount,
	const char *const *pipelines,
	const char *mesh,
	uint32_t instanceCount,
	uint32_t instanceOffset
);

void endRender(const vk::Device &device, const vk::Queue &queue);

void terminate(const vk::Device &device);

} // namespace graphics::rendering
