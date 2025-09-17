#pragma once

#include "../utils.hpp"

namespace graphics::resource {

class Buffer {
private:
	const bool _isStorage;
	const vk::DeviceSize _size;
	const vk::Buffer _buffer;
	const vk::DeviceMemory _memory;

public:
	Buffer() = delete;
	Buffer(const Buffer &) = delete;
	Buffer &operator =(const Buffer &) = delete;

	Buffer(uint64_t size, bool isStorage, bool isHostCoherent);
	~Buffer();

	const vk::Buffer &get() const noexcept {
		return _buffer;
	}

	bool isStorage() const noexcept {
		return _isStorage;
	}

	template<typename T>
	void update(const T *data) const {
		copyDataToMemory(_memory, data, _size);
	}

	template<typename T>
	void update(const T *data, size_t size, size_t offset) const {
		copyDataToMemory(_memory, data, size, offset);
	}

	template<typename T>
	void copyTo(T *data) const {
		copyMemotyToData(data, _memory, _size);
	}
};

void destroyAllBuffers() noexcept;

void addBuffer(const std::string &id, uint64_t size, bool isStorage, bool isHostCoherent);

void destroyBuffer(const std::string &id) noexcept;

const Buffer &getBuffer(const std::string &id);

} // namespace graphics::resource
