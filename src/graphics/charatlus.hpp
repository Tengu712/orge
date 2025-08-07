#pragma once

#include "image.hpp"

#include <memory>

namespace graphics {

class CharAtlus {
private:
	const uint32_t _colCount;
	const uint32_t _rowCount;
	Image _image;

	CharAtlus(
		const vk::PhysicalDeviceMemoryProperties &memoryProps,
		const vk::Device &device,
		const vk::Queue &queue
	);

	void _put(
		uint32_t codepoint,
		const vk::PhysicalDeviceMemoryProperties &memoryProps,
		const vk::Device &device,
		const vk::Queue &queue
	);

public:
	CharAtlus() = delete;
	CharAtlus(const CharAtlus &) = delete;
	CharAtlus(const CharAtlus &&) = delete;
	CharAtlus &operator =(const CharAtlus &) = delete;
	CharAtlus &operator =(const CharAtlus &&) = delete;

	static std::unique_ptr<CharAtlus> create(
		const vk::PhysicalDeviceMemoryProperties &memoryProps,
		const vk::Device &device,
		const vk::Queue &queue
	);

	void destroy(const vk::Device &device) const noexcept {
		_image.destroy(device);
	}

	const Image &get() const noexcept {
		return _image;
	}
};

} // namespace graphics
