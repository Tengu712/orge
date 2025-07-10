#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics::utils {

uint32_t findMemoryType(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	uint32_t typeBits,
	vk::MemoryPropertyFlags mask
);

vk::DeviceMemory allocateMemory(
	const vk::Device &device,
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Buffer &buffer,
	vk::MemoryPropertyFlags mask
);

} // namespace graphics::utils
