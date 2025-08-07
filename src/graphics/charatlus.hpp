#pragma once

#include "image.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace graphics {

class CharAtlus {
private:
	const uint32_t _colCount;
	const uint32_t _rowCount;
	Image _image;
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

	void destroyFont(const std::string &id) noexcept {
		if (_fonts.contains(id)) {
			_fonts.erase(id);
		}
	}
};

} // namespace graphics
