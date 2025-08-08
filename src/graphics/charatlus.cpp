#include "charatlus.hpp"

#include "../config/config.hpp"

#include <format>
#include <fstream>
#include <memory>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include <utf8cpp/utf8.h>

namespace graphics {

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

void freeBitmap(unsigned char *p) {
    stbtt_FreeBitmap(p, nullptr);
}

void putCharacter(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Queue &queue,
	const config::FontConfig &config,
	Image &dst,
	CharLru &chars,
	const stbtt_fontinfo &fontinfo,
	uint32_t codepoint
) {
	using Pixels = std::unique_ptr<unsigned char, decltype(&freeBitmap)>;

	int w, h, x, y;
	const auto scale = stbtt_ScaleForPixelHeight(&fontinfo, static_cast<float>(config.charSize));
	const auto bitmap = Pixels(
		stbtt_GetCodepointBitmap(&fontinfo, scale, scale, static_cast<int>(codepoint), &w, &h, &x, &y),
		freeBitmap
	);
	if (w <= 0 || h <= 0) {
		throw std::format("failed to rasterize the character whose codepoint is {}.", codepoint);
	}

	uint32_t offsetX, offsetY;
	if (!chars.popOldestIfSaturated(offsetX, offsetY)) {
		const auto n = chars.size();
		offsetX = n % config.charAtlusCol * config.charSize;
		offsetY = n / config.charAtlusRow * config.charSize;
	}

	dst.upload(
		memoryProps,
		device,
		queue,
		static_cast<uint32_t>(w),
		static_cast<uint32_t>(h),
		offsetX,
		offsetY,
		reinterpret_cast<uint8_t *>(bitmap.get()),
		true
	);

	chars.put(codepoint, Character(offsetX, offsetY));
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

} // namespace graphics
