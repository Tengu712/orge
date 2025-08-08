#include "charatlus.hpp"

#include "../error/error.hpp"
#include "../config/config.hpp"

#include <fstream>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include <utf8cpp/utf8.h>

namespace graphics {

void freeBitmap(unsigned char *p) {
    stbtt_FreeBitmap(p, nullptr);
}

void putCharacter(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Queue &queue,
	Image &dst,
	CharLru &chars,
	const stbtt_fontinfo &fontinfo,
	uint32_t codepoint
) {
	using Pixels = std::unique_ptr<unsigned char, decltype(&freeBitmap)>;

	int w, h, x, y;
	const auto scale = stbtt_ScaleForPixelHeight(&fontinfo, static_cast<float>(config::config().charSize));
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
		offsetX = n % config::config().charAtlusCol * config::config().charSize;
		offsetY = n / config::config().charAtlusRow * config::config().charSize;
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
	const vk::Queue &queue
) :
	_image(
		memoryProps,
		device,
		queue,
		config::config().charAtlusCol * config::config().charSize,
		config::config().charAtlusRow * config::config().charSize,
		std::vector<uint8_t>(
			config::config().charAtlusCol * config::config().charSize
				* config::config().charAtlusRow * config::config().charSize
		).data(),
		true
	),
	_chars(config::config().charAtlusCol * config::config().charAtlusRow)
{
	// DEBUG:
	loadFontFromFile("font", "MPLUS1p-Regular.ttf");
	stbtt_fontinfo info;
	if (!stbtt_InitFont(&info, _fonts.at("font").data(), 0)) {
		throw std::format("failed to get the information of '{}'.", "font");
	}
	const std::string s = "あいあう";
	auto itr = s.cbegin();
	auto end = s.cend();
	while (itr != end) {
		const auto codepoint = utf8::next(itr, end);
		if (!_chars.has(codepoint)) {
			putCharacter(memoryProps, device, queue, _image, _chars, info, codepoint);
		}
	}
}

std::unique_ptr<CharAtlus> CharAtlus::create(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Queue &queue
) {
	if (config::config().charSize > 0 && config::config().charAtlusCol > 0 && config::config().charAtlusRow > 0) {
		return std::unique_ptr<CharAtlus>(new CharAtlus(memoryProps, device, queue));
	} else {
		return nullptr;
	}
}

void CharAtlus::loadFontFromFile(const std::string &id, const std::string &path) {
	std::ifstream file(path, std::ios::binary);
	if (!file) {
		throw std::format("failed to load '{}'.", path);
	}

	file.seekg(0, std::ios::end);
	const auto size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<unsigned char> buffer(size);
	file.read(reinterpret_cast<char *>(buffer.data()), size);

	_fonts.emplace(id, buffer);
}

} // namespace graphics
