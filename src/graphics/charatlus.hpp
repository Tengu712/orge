#pragma once

#include "../config/config.hpp"
#include "charlru.hpp"
#include "image.hpp"

#include <stb_truetype.h>
#include <vector>

namespace graphics {

class CharAtlus {
private:
	const config::FontConfig &_config;
	const std::vector<unsigned char> _font;
	const stbtt_fontinfo _fontinfo;
	const float _scale;
	const float _ascent;
	const float _lineAdvance;
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

	float getRangeOfU() const noexcept {
		return static_cast<float>(_config.charSize) / _width;
	}

	float getRangeOfV() const noexcept {
		return static_cast<float>(_config.charSize) / _height;
	}

	float calcMeshSize(float height) const noexcept {
		const auto scale = height / static_cast<float>(_config.charSize);
		return static_cast<float>(_config.charSize) * scale;
	}

	float calcAscent(float height) const noexcept {
		const auto scale = height / static_cast<float>(_config.charSize);
		return _ascent * scale;
	}

	float getLineAdvance() const noexcept {
		return _lineAdvance;
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
