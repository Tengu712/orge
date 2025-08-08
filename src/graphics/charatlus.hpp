#pragma once

#include "charlru.hpp"
#include "image.hpp"

#include <memory>
#include <string>
#include <vector>

namespace graphics {

class CharAtlus {
private:
	const std::vector<unsigned char> _font;
	Image _image;
	CharLru _chars;

	CharAtlus(
		const vk::PhysicalDeviceMemoryProperties &memoryProps,
		const vk::Device &device,
		const vk::Queue &queue,
		const std::string &path
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
		const vk::Queue &queue,
		const std::string &path
	);

	void destroy(const vk::Device &device) const noexcept {
		_image.destroy(device);
	}

	const Image &get() const noexcept {
		return _image;
	}
};

} // namespace graphics
