#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics {

void terminateUtils(const vk::Device &device);

void initializeUtils(const vk::Device &device, const vk::CommandPool &commandPool);

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

inline vk::DeviceMemory allocateBufferMemory(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Buffer &buffer,
	vk::MemoryPropertyFlags mask
) {
	const auto reqs = device.getBufferMemoryRequirements(buffer);
	const auto type = findMemoryType(memoryProps, reqs.memoryTypeBits, mask);
	const auto memory = device.allocateMemory(vk::MemoryAllocateInfo(reqs.size, type));
	device.bindBufferMemory(buffer, memory, 0);
	return memory;
}

inline vk::DeviceMemory allocateImageMemory(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Image &image,
	vk::MemoryPropertyFlags mask
) {
	const auto reqs = device.getImageMemoryRequirements(image);
	const auto type = findMemoryType(memoryProps, reqs.memoryTypeBits, mask);
	const auto memory = device.allocateMemory(vk::MemoryAllocateInfo(reqs.size, type));
	device.bindImageMemory(image, memory, 0);
	return memory;
}

template<typename T>
inline void copyDataToMemory(const vk::Device &device, const vk::DeviceMemory &dst, const T *src, size_t size) {
	const auto p = static_cast<T *>(device.mapMemory(dst, 0, size));
	memcpy(p, src, size);
	device.unmapMemory(dst);
}

void uploadBuffer(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Queue &queue,
	const vk::Buffer &dst,
	const void *src,
	size_t size,
	vk::PipelineStageFlags visibleStages
);

void uploadImage(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Queue &queue,
	const vk::Image &dst,
	uint32_t width,
	uint32_t height,
	uint32_t channels,
	uint32_t offsetX,
	uint32_t offsetY,
	const uint8_t *src
);

} // namespace graphics
