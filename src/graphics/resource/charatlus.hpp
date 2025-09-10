#pragma once

#include "charatlus-char.hpp"
#include "charatlus-lru.hpp"
#include "image.hpp"

#include <optional>
#include <stb_truetype.h>
#include <string>

namespace graphics::resource {

class CharAtlus: public Image {
private:
	const std::string _id;
	const stbtt_fontinfo _fontinfo;
	const uint32_t _size;
	const uint32_t _width;
	const uint32_t _height;
	const float _scale;
	const float _ascent;
	const float _lineAdvance;
	const float _widthf;
	const float _heightf;
	CharLru _chars;

	float _scaled(float n, float height) const noexcept;

public:
	CharAtlus() = delete;
	CharAtlus(const CharAtlus &) = delete;
	CharAtlus &operator =(const CharAtlus &) = delete;

	CharAtlus(const std::string &id);
	~CharAtlus() {}

	std::optional<ScaledCharacter> getScaledCharacter(uint32_t codepoint, float height) noexcept {
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

	float getRangeOfU() const noexcept;
	float getRangeOfV() const noexcept;
	float calcMeshSize(float height) const noexcept;
	float calcAscent(float height) const noexcept;
	float calcLineAdvance(float height) const noexcept;

	void rasterizeCharacters(const std::string &s);
};

void destroyAllCharAtluses() noexcept;

void initializeAllCharAtluses();

CharAtlus &getCharAtlus(const std::string &id);

} // namespace graphics::resource
