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

CharAtlus::CharAtlus(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Queue &queue,
	const config::FontConfig &config
) :
	_config(config),
	// TODO: フォントデータからも作成できるように
	_font(loadFontFromFile(config.path.value())),
	_width(static_cast<float>(_config.charAtlusCol * _config.charSize)),
	_height(static_cast<float>(_config.charAtlusRow * _config.charSize)),
	_image(
		memoryProps,
		device,
		queue,
		_config.charAtlusCol * _config.charSize,
		_config.charAtlusRow * _config.charSize,
		std::vector<uint8_t>(_config.charAtlusCol * _config.charSize * _config.charAtlusRow * _config.charSize).data(),
		true
	),
	_chars(_config.charAtlusCol * _config.charAtlusRow)
{}

void CharAtlus::putString(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Queue &queue,
	const std::string &s
) {
	using Pixels = std::unique_ptr<unsigned char, decltype(&freeBitmap)>;

	stbtt_fontinfo info;
	if (!stbtt_InitFont(&info, _font.data(), 0)) {
		throw "failed to get a font information.";
	}

	auto itr = s.begin();
	auto end = s.end();
	while (itr != end) {
		const auto codepoint = static_cast<uint32_t>(utf8::next(itr, end));
		if (_chars.has(codepoint)) {
			continue;
		}

		// ラスタライズ
		int w, h, ox, oy;
		const auto scale = stbtt_ScaleForPixelHeight(&info, static_cast<float>(_config.charSize));
		const auto bitmap = Pixels(
			stbtt_GetCodepointBitmap(&info, scale, scale, static_cast<int>(codepoint), &w, &h, &ox, &oy),
			freeBitmap
		);
		if (w <= 0 || h <= 0) {
			// TODO: 無効字は空白にする
			throw std::format("failed to rasterize the character whose codepoint is {}.", codepoint);
		}

		// 文字送り幅取得
		int advance;
		stbtt_GetCodepointHMetrics(&info, codepoint, &advance, nullptr);
		advance *= scale;

		// 飽和状態なら最古を消してその位置へ・そうでないなら次の位置へ
		uint32_t x, y;
		if (!_chars.popOldestIfSaturated(x, y)) {
			const auto n = _chars.size();
			x = n % _config.charAtlusCol * _config.charSize;
			y = n / _config.charAtlusCol * _config.charSize;
		}

		// アップロード
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

		// 登録
		_chars.put(codepoint, Character(
			x,
			y,
			static_cast<float>(ox),
			static_cast<float>(oy),
			static_cast<float>(advance),
			static_cast<float>(x) / _width,
			static_cast<float>(y) / _height
		));
	}
}

} // namespace graphics
