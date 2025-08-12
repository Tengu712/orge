#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics {

class Mesh {
private:
	const uint32_t _iCount;
	const vk::Buffer _vb;
	const vk::Buffer _ib;
	const vk::DeviceMemory _vbMemory;
	const vk::DeviceMemory _ibMemory;

public:
	Mesh() = delete;
	Mesh(
		const vk::PhysicalDeviceMemoryProperties &memoryProps,
		const vk::Device &device,
		const vk::Queue &queue,
		const std::string &id
	);

	void destroy(const vk::Device &device) const noexcept {
		device.freeMemory(_ibMemory);
		device.freeMemory(_vbMemory);
		device.destroyBuffer(_ib);
		device.destroyBuffer(_vb);
	}

	uint32_t getIndexCount() const noexcept {
		return _iCount;
	}

	void bind(const vk::CommandBuffer &commandBuffer) const noexcept {
		const VkDeviceSize offset = 0;
		commandBuffer.bindVertexBuffers(0, 1, &_vb, &offset);
		commandBuffer.bindIndexBuffer(_ib, offset, vk::IndexType::eUint32);
	}
};

} // namespace graphics
