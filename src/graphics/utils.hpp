#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics::utils {

inline uint32_t findMemoryType(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	uint32_t typeBits,
	vk::MemoryPropertyFlags mask
) {
	uint32_t result = UINT32_MAX;

	for (uint32_t i = 0; i < memoryProps.memoryTypeCount; ++i) {
		if ((typeBits & 1) && ((memoryProps.memoryTypes[i].propertyFlags & mask) == mask)) {
			result = i;
			break;
		}
		typeBits >>= 1;
	}

	if (result == UINT32_MAX) {
		throw "failed to find a correct memory type.";
	}
	return result;
}

inline vk::DeviceMemory allocateMemory(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Buffer &buffer,
	vk::MemoryPropertyFlags mask
) {
	const auto reqs = device.getBufferMemoryRequirements(buffer);
	const auto type = utils::findMemoryType(memoryProps, reqs.memoryTypeBits, mask);
	const auto memory = device.allocateMemory(vk::MemoryAllocateInfo(reqs.size, type));
	device.bindBufferMemory(buffer, memory, 0);
	return memory;
}

} // namespace graphics::utils
