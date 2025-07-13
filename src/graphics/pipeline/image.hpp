#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics::pipeline::image {

struct Image {
	vk::Image image;
	vk::ImageView view;
	vk::DeviceMemory memory;
};

void terminate(const vk::Device &device);

void initialize(const vk::Device &device, const vk::CommandPool &commandPool);

void createFromFile(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Queue &queue,
	const char *id,
	const char *path
);

} // namespace graphics::pipeline::image
