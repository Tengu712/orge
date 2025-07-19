#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics::rendering::mesh {

void terminate(const vk::Device &device);

void createMesh(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const char *id,
	const uint32_t vertexCount,
	const float *vertices,
	const uint32_t indexCount,
	const uint32_t *indices
);

void destroy(const vk::Device &device, const char *id);

/// メッシュをバインドする関数
///
/// バインドされたメッシュのインデックスカウントを返す。
uint32_t bind(const vk::CommandBuffer &commandBuffer, const char *id);

} // namespace graphics::rendering::mesh
