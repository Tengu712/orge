#include "charatlus.hpp"

#include "../../asset/asset.hpp"
#include "../../config/config.hpp"
#include "../../error/error.hpp"

#include <memory>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include <utf8cpp/utf8.h>

namespace graphics::resource {

void freeBitmap(unsigned char *p) {
    stbtt_FreeBitmap(p, nullptr);
}

stbtt_fontinfo createFontInfo(const std::string &id) {
	const auto &file = config::config().fonts.at(id).file;
	const auto assetId = error::at(config::config().assetMap, file, "assets");
	const auto font = asset::getAsset(assetId);
	stbtt_fontinfo info;
	if (stbtt_InitFont(&info, font.data(), 0)) {
		return info;
	} else {
		throw std::format("failed to load font file: {}", file);
	}
}

float getScale(const stbtt_fontinfo &fontinfo, uint32_t size) {
	return stbtt_ScaleForPixelHeight(&fontinfo, static_cast<float>(size));
}

float getAscentFrom(const stbtt_fontinfo &fontinfo, float scale) {
	int ascent, decent, lineGap;
	stbtt_GetFontVMetrics(&fontinfo, &ascent, &decent, &lineGap);
	return static_cast<float>(ascent) * scale;
}

float getLineAdvanceFrom(const stbtt_fontinfo &fontinfo, float scale) {
	int ascent, decent, lineGap;
	stbtt_GetFontVMetrics(&fontinfo, &ascent, &decent, &lineGap);
	return static_cast<float>(ascent - decent + lineGap) * scale;
}

uint32_t getWidth(const std::string &id) {
	const auto &cfg = config::config().fonts.at(id);
	return cfg.charAtlusCol * (cfg.charSize + 1);
}

uint32_t getHeight(const std::string &id) {
	const auto &cfg = config::config().fonts.at(id);
	return cfg.charAtlusRow * (cfg.charSize + 1);
}

uint32_t getMaxCount(const std::string &id) {
	const auto &cfg = config::config().fonts.at(id);
	return cfg.charAtlusCol * cfg.charAtlusRow;
}

CharAtlus::CharAtlus(const std::string &id):
	Image(
		getWidth(id),
		getHeight(id),
		std::vector<uint8_t>(getWidth(id) * getHeight(id)).data(),
		vk::Format::eR8Unorm,
		vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
		vk::ImageAspectFlagBits::eColor,
		1
	),
	_id(id),
	_fontinfo(createFontInfo(id)),
	_size(config::config().fonts.at(id).charSize),
	_width(getWidth(id)),
	_height(getHeight(id)),
	_scale(getScale(_fontinfo, _size)),
	_ascent(getAscentFrom(_fontinfo, _scale)),
	_lineAdvance(getLineAdvanceFrom(_fontinfo, _scale)),
	_widthf(static_cast<float>(_width)),
	_heightf(static_cast<float>(_height)),
	_chars(getMaxCount(id))
{}

float CharAtlus::_scaled(float n, float height) const noexcept {
	return n * height / static_cast<float>(config::config().fonts.at(_id).charSize);
}

float CharAtlus::getRangeOfU() const noexcept {
	return static_cast<float>(config::config().fonts.at(_id).charSize) / _widthf;
}

float CharAtlus::getRangeOfV() const noexcept {
	return static_cast<float>(config::config().fonts.at(_id).charSize) / _heightf;
}

float CharAtlus::calcMeshSize(float height) const noexcept {
	return _scaled(static_cast<float>(config::config().fonts.at(_id).charSize), height);
}

float CharAtlus::calcAscent(float height) const noexcept {
	return _scaled(_ascent, height);
}

float CharAtlus::calcLineAdvance(float height) const noexcept {
	return _scaled(_lineAdvance, height);
}

void CharAtlus::rasterizeCharacters(const std::string &s) {
	using Pixels = std::unique_ptr<unsigned char, decltype(&freeBitmap)>;

	auto itr = s.begin();
	auto end = s.end();
	while (itr != end) {
		const auto codepoint = static_cast<uint32_t>(utf8::next(itr, end));
		// NOTE: LFとCRもスキップする。
		if (_chars.has(codepoint) || codepoint == 10 || codepoint == 13) {
			continue;
		}

		// ラスタライズ
		int w, h, ox, oy;
		const auto bitmap = Pixels(
			stbtt_GetCodepointBitmap(&_fontinfo, _scale, _scale, static_cast<int>(codepoint), &w, &h, &ox, &oy),
			freeBitmap
		);

		// ラスタライズ先の左上座標を取得
		const auto &cfg = config::config().fonts.at(_id);
		auto x = _chars.size() % cfg.charAtlusCol * (cfg.charSize + 1);
		auto y = _chars.size() / cfg.charAtlusCol * (cfg.charSize + 1);
		_chars.popOldestIfSaturated(x, y);

		// 有効な字に限り、送り幅を取得、アップロード
		int advance = _size;
		if (bitmap && w > 0 && h > 0) {
			stbtt_GetCodepointHMetrics(&_fontinfo, codepoint, &advance, nullptr);

			upload(
				static_cast<uint32_t>(w),
				static_cast<uint32_t>(h),
				x,
				y,
				reinterpret_cast<uint8_t *>(bitmap.get())
			);
		}
		// 無効な字なら安全のために情報をリセット
		else {
			w = 0;
			ox = 0;
			oy = 0;
		}

		// 登録
		_chars.put(codepoint, Character(
			x,
			y,
			static_cast<float>(w),
			static_cast<float>(ox),
			static_cast<float>(oy),
			static_cast<float>(advance) * _scale,
			static_cast<float>(x) / _widthf,
			static_cast<float>(y) / _heightf
		));
	}
}

std::unordered_map<std::string, CharAtlus> g_charAtluses;

void destroyAllCharAtluses() noexcept {
	g_charAtluses.clear();
}

void initializeAllCharAtluses() {
	if (!g_charAtluses.empty()) {
		throw "char atluses already initlaized.";
	}
	for (const auto &[id, _]: config::config().fonts) {
		g_charAtluses.emplace(id, id);
	}
}

CharAtlus &getCharAtlus(const std::string &id) {
	return error::atMut(g_charAtluses, id, "char atluses");
}

const std::unordered_map<std::string, CharAtlus> &charAtluses() noexcept {
	return g_charAtluses;
}

} // namespace graphics::resource
