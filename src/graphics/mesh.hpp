#pragma once

#include <string>
#include <vulkan/vulkan.hpp>

namespace graphics::mesh {

void createMesh(
	const vk::PhysicalDeviceMemoryProperties &physicalDeviceMemoryProps,
	const vk::Device &device,
	const char *id,
	const unsigned int vertexCount,
	const float *vertices,
	const unsigned int indexCount,
	const float *indices
);

void terminate(const vk::Device &device);

} // namespace graphics::mesh
