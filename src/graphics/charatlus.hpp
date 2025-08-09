#pragma once

#include "../config/config.hpp"
#include "charlru.hpp"
#include "image.hpp"

#include <vector>

namespace graphics {

class CharAtlus {
private:
	const config::FontConfig &_config;
	const std::vector<unsigned char> _font;
	const float _width;
	const float _height;
	Image _image;
	CharLru _chars;

public:
	CharAtlus() = delete;
	CharAtlus(
		const vk::PhysicalDeviceMemoryProperties &memoryProps,
		const vk::Device &device,
		const vk::Queue &queue,
		const config::FontConfig &config
	);

	void destroy(const vk::Device &device) const noexcept {
		_image.destroy(device);
	}

	const Image &get() const noexcept {
		return _image;
	}

	void putString(
		const vk::PhysicalDeviceMemoryProperties &memoryProps,
		const vk::Device &device,
		const vk::Queue &queue,
		const std::string &s
	);
};

} // namespace graphics
