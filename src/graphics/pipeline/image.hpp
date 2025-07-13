#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics::pipeline::image {

struct Image {
	const vk::Image image;
	const vk::ImageView view;
	const vk::DeviceMemory memory;
	const vk::Sampler &sampler;
};

void terminate(const vk::Device &device);

void initialize(const vk::Device &device, const vk::CommandPool &commandPool);

void createFromFile(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Queue &queue,
	const char *id,
	const char *path,
	int linearMagFilter,
	int linearMinFilter,
	int repeat
);

} // namespace graphics::pipeline::image
