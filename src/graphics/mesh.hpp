#pragma once

#include "utils.hpp"

#define VB_SIZE() (sizeof(float)    * vertexCount)
#define IB_SIZE() (sizeof(uint32_t) * indexCount)

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
		const uint32_t vertexCount,
		const float *vertices,
		const uint32_t indexCount,
		const uint32_t *indices
	) :
		_iCount(indexCount),
		_vb(device.createBuffer(
			vk::BufferCreateInfo()
				.setSize(VB_SIZE())
				.setUsage(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst)
		)),
		_ib(device.createBuffer(
			vk::BufferCreateInfo()
				.setSize(IB_SIZE())
				.setUsage(vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst)
		)),
		_vbMemory(allocateBufferMemory(memoryProps, device, _vb, vk::MemoryPropertyFlagBits::eDeviceLocal)),
		_ibMemory(allocateBufferMemory(memoryProps, device, _ib, vk::MemoryPropertyFlagBits::eDeviceLocal))
	{
		uploadBuffer(
			memoryProps,
			device,
			queue,
			_vb,
			static_cast<const void *>(vertices),
			VB_SIZE(),
			vk::PipelineStageFlagBits::eVertexShader
		);
		uploadBuffer(
			memoryProps,
			device,
			queue,
			_ib,
			static_cast<const void *>(indices),
			IB_SIZE(),
			vk::PipelineStageFlagBits::eVertexShader
		);
	}

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

#undef IB_SIZE
#undef VB_SIZE
