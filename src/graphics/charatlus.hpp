#pragma once

#include "charlru.hpp"
#include "image.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace graphics {

class CharAtlus {
private:
	Image _image;
	CharLru _chars;
	std::unordered_map<std::string, std::vector<unsigned char>> _fonts;

	CharAtlus(
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

	void loadFontFromFile(const std::string &id, const std::string &path);
};

} // namespace graphics
