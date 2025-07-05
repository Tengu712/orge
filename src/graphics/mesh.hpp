#pragma once

#include <string>
#include <vulkan/vulkan.hpp>

namespace graphics::mesh {

void createMesh(
	const vk::PhysicalDeviceMemoryProperties &physicalDeviceMemoryProps,
	const vk::Device &device,
	const char *id,
	const uint32_t vertexCount,
	const float *vertices,
	const uint32_t indexCount,
	const uint32_t *indices
);

/// メッシュをバインドする関数
///
/// バインドされたメッシュのインデックスカウントを返す。
uint32_t bind(const vk::CommandBuffer &commandBuffer, const char *id);

void terminate(const vk::Device &device);

} // namespace graphics::mesh
