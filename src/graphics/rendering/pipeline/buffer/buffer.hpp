#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics::rendering::pipeline::buffer {

void terminate(const vk::Device &device);

struct Buffer {
	const bool isStorage;
	const vk::DeviceSize size;
	const vk::Buffer buffer;
	const vk::DeviceMemory memory;
};

void create(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const char *id,
	uint64_t size,
	int isStorage
);

void update(const vk::Device &device, const char *id, const void *data);

const Buffer &get(const char *id);

} // namespace graphics::rendering::pipeline::buffer
