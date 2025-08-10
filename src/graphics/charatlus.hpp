#pragma once

#include "../config/config.hpp"
#include "charlru.hpp"
#include "image.hpp"

#include <optional>
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

	float _scaled(float n, float height) const noexcept {
		return n * height / static_cast<float>(_config.charSize);
	}

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

	const std::optional<ScaledCharacter> getScaledCharacter(uint32_t codepoint, float height) noexcept {
		if (_chars.has(codepoint)) {
			const auto &c = _chars.use(codepoint);
			return std::make_optional<ScaledCharacter>(
				_scaled(c.w, height),
				_scaled(c.ox, height),
				_scaled(c.oy, height),
				_scaled(c.advance, height),
				c.u,
				c.v
			);
		} else {
			return std::nullopt;
		}
	}

	float getRangeOfU() const noexcept {
		return static_cast<float>(_config.charSize) / _width;
	}

	float getRangeOfV() const noexcept {
		return static_cast<float>(_config.charSize) / _height;
	}

	float calcMeshSize(float height) const noexcept {
		return _scaled(static_cast<float>(_config.charSize), height);
	}

	float calcAscent(float height) const noexcept {
		return _scaled(_ascent, height);
	}

	float calcLineAdvance(float height) const noexcept {
		return _scaled(_lineAdvance, height);
	}

	void putString(
		const vk::PhysicalDeviceMemoryProperties &memoryProps,
		const vk::Device &device,
		const vk::Queue &queue,
		const std::string &s
	);
};

} // namespace graphics
