#pragma once

#include "../utils.hpp"

namespace graphics::resource {

class Buffer {
private:
	const bool _isStorage;
	const vk::DeviceSize _size;
	const vk::UniqueBuffer _buffer;
	const vk::UniqueDeviceMemory _memory;

public:
	Buffer() = delete;
	Buffer(const Buffer &) = delete;
	Buffer &operator =(const Buffer &) = delete;

	Buffer(uint64_t size, bool isStorage, bool isHostCoherent);

	const vk::Buffer &get() const noexcept {
		return _buffer.get();
	}

	bool isStorage() const noexcept {
		return _isStorage;
	}

	template<typename T>
	void update(const T *data) const {
		copyDataToMemory(_memory.get(), data, _size);
	}

	template<typename T>
	void update(const T *data, size_t size, size_t offset) const {
		copyDataToMemory(_memory.get(), data, size, offset);
	}

	template<typename T>
	void copyTo(T *data) const {
		copyMemotyToData(data, _memory.get(), _size);
	}
};

void destroyAllBuffers() noexcept;

void addBuffer(const std::string &id, uint64_t size, bool isStorage, bool isHostCoherent);

void destroyBuffer(const std::string &id) noexcept;

const Buffer &getBuffer(const std::string &id);

} // namespace graphics::resource
