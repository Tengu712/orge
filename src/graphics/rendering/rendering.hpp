#pragma once

#include "../../config/config.hpp"

#include <vulkan/vulkan.hpp>

namespace graphics::rendering {

void terminate(const vk::Instance &instance, const vk::Device &device);

void initialize(
	const config::Config &config,
	const vk::Instance &instance,
	const vk::PhysicalDevice &physicalDevice,
	const vk::Device &device,
	const vk::CommandPool &commandPool
);

void beginRender(const vk::Device &device);

void endRender(const vk::Queue &queue);

void draw(
	const vk::Device &device,
	const char *pipelineId,
	const char *meshId,
	uint32_t instanceCount,
	uint32_t instanceOffset
);

} // namespace graphics::rendering
