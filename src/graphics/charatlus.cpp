#include "charatlus.hpp"

#include "../config/config.hpp"

#include <format>
#include <fstream>
#include <memory>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include <utf8cpp/utf8.h>

namespace graphics {

void freeBitmap(unsigned char *p) {
    stbtt_FreeBitmap(p, nullptr);
}

std::vector<unsigned char> loadFontFromFile(const std::string &path) {
	std::ifstream file(path, std::ios::binary);
	if (!file) {
		throw std::format("failed to load '{}'.", path);
	}

	file.seekg(0, std::ios::end);
	const auto size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<unsigned char> buffer(size);
	file.read(reinterpret_cast<char *>(buffer.data()), size);

	return buffer;
}

stbtt_fontinfo createFontInfo(const std::vector<unsigned char> &font) {
	stbtt_fontinfo info;
	if (stbtt_InitFont(&info, font.data(), 0)) {
		return info;
	} else {
		throw "failed to get a font information.";
	}
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

CharAtlus::CharAtlus(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Queue &queue,
	const config::FontConfig &config
) :
	_config(config),
	// TODO: フォントデータからも作成できるように
	_font(loadFontFromFile(config.path.value())),
	_fontinfo(createFontInfo(_font)),
	_scale(stbtt_ScaleForPixelHeight(&_fontinfo, static_cast<float>(_config.charSize))),
	_ascent(getAscentFrom(_fontinfo, _scale)),
	_lineAdvance(getLineAdvanceFrom(_fontinfo, _scale)),
	_width(static_cast<float>(_config.charAtlusCol * (_config.charSize + 1))),
	_height(static_cast<float>(_config.charAtlusRow * (_config.charSize + 1))),
	_image(
		memoryProps,
		device,
		queue,
		_config.charAtlusCol * (_config.charSize + 1),
		_config.charAtlusRow * (_config.charSize + 1),
		std::vector<uint8_t>(
			_config.charAtlusCol * (_config.charSize + 1) * _config.charAtlusRow * (_config.charSize + 1)
		).data(),
		true
	),
	_chars(_config.charAtlusCol * _config.charAtlusRow)
{}

void CharAtlus::rasterizeCharacters(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Queue &queue,
	const std::string &s
) {
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
		auto x = _chars.size() % _config.charAtlusCol * (_config.charSize + 1);
		auto y = _chars.size() / _config.charAtlusCol * (_config.charSize + 1);
		_chars.popOldestIfSaturated(x, y);

		// 有効な字に限り、送り幅を取得、アップロード
		int advance = _config.charSize;
		if (bitmap && w > 0 && h > 0) {
			stbtt_GetCodepointHMetrics(&_fontinfo, codepoint, &advance, nullptr);

			_image.upload(
				memoryProps,
				device,
				queue,
				static_cast<uint32_t>(w),
				static_cast<uint32_t>(h),
				x,
				y,
				reinterpret_cast<uint8_t *>(bitmap.get()),
				true
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
			static_cast<float>(x) / _width,
			static_cast<float>(y) / _height
		));
	}
}

} // namespace graphics
