#pragma once

#include "utils.hpp"

namespace graphics {

class Buffer {
private:
	const bool _isStorage;
	const vk::DeviceSize _size;
	const vk::Buffer _buffer;
	const vk::DeviceMemory _memory;

public:
	Buffer() = delete;
	Buffer(const vk::PhysicalDeviceMemoryProperties &memoryProps, const vk::Device &device, uint64_t size, int isStorage) :
		_isStorage(static_cast<bool>(isStorage)),
		_size(static_cast<vk::DeviceSize>(size)),
		_buffer(device.createBuffer(
			vk::BufferCreateInfo()
				.setSize(size)
				.setUsage(_isStorage ? vk::BufferUsageFlagBits::eStorageBuffer : vk::BufferUsageFlagBits::eUniformBuffer)
				.setSharingMode(vk::SharingMode::eExclusive)
		)),
		_memory(allocateBufferMemory(
			memoryProps,
			device,
			_buffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		))
	{}

	void destroy(const vk::Device &device) const noexcept {
		device.freeMemory(_memory);
		device.destroyBuffer(_buffer);
	}

	const vk::Buffer &get() const noexcept {
		return _buffer;
	}

	bool isStorage() const noexcept {
		return _isStorage;
	}

	void update(const vk::Device &device, const void *data) const {
		copyDataToMemory(device, _memory, data, _size);
	}
};

} // namespace graphics
