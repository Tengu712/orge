#pragma once

#include "../../../config/config.hpp"

#include <vulkan/vulkan.hpp>

namespace graphics::rendering::framebuffer {

void terminate(const vk::Device &device);

void initialize(
	const config::Config &config,
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::RenderPass &renderPass,
	const vk::Extent2D &extent,
	const std::vector<vk::Image> &swapchainImages
);

const std::vector<vk::ClearValue> &getClearValues();

const vk::Framebuffer &getFramebuffer(uint32_t index);

} // namespace graphics::rendering::framebuffer
