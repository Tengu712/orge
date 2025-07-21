#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics::rendering::pipeline::image {

struct Image {
	const vk::Image image;
	const vk::ImageView view;
	const vk::DeviceMemory memory;
};

void terminate(const vk::Device &device);

void initialize(const vk::Device &device, const vk::CommandPool &commandPool);

void create(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Queue &queue,
	const char *id,
	uint32_t width,
	uint32_t height,
	const unsigned char *pixels
);

void createFromFile(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Queue &queue,
	const char *id,
	const char *path
);

void destroy(const vk::Device &device, const char *id);

const Image &get(const char *id);

} // namespace graphics::rendering::pipeline::image
