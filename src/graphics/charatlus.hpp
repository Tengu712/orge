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

	const Character *getCharacter(uint32_t codepoint) noexcept {
		if (_chars.has(codepoint)) {
			return &_chars.use(codepoint);
		} else {
			return nullptr;
		}
	}

	void getCharacterCommonInfo(float &size, float &ru, float &rv) const noexcept {
		size = static_cast<float>(_config.charSize);
		ru = static_cast<float>(_config.charSize) / _width;
		rv = static_cast<float>(_config.charSize) / _height;
	}

	float calcScale(float height) const noexcept {
		return height / static_cast<float>(_config.charSize);
	}

	void putString(
		const vk::PhysicalDeviceMemoryProperties &memoryProps,
		const vk::Device &device,
		const vk::Queue &queue,
		const std::string &s
	);
};

} // namespace graphics
