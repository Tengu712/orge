#include "charatlus.hpp"

// DEBUG:
#include <iostream>

#include "../config/config.hpp"

#include <utf8cpp/utf8.h>
#include <vector>

namespace graphics {

void CharAtlus::_put(
	uint32_t codepoint,
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Queue &queue
) {
	// DEBUG:
	std::vector<uint8_t> foo(16 * 24, 255);
	_image.upload(memoryProps, device, queue, 16, 24, 20, 10, foo.data(), true);
	std::cout << codepoint << std::endl;
}

CharAtlus::CharAtlus(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Queue &queue
) :
	_colCount(config::config().charAtlusCol),
	_rowCount(config::config().charAtlusRow),
	_image(
		memoryProps,
		device,
		queue,
		_colCount * config::config().charSize,
		_rowCount * config::config().charSize,
		std::vector<uint8_t>(_colCount * config::config().charSize * _rowCount * config::config().charSize).data(),
		true
	)
{
	// DEBUG:
	const std::string s = "あいう";
	auto itr = s.cbegin();
	auto end = s.cend();
	while (itr != end) {
		const auto codepoint = utf8::next(itr, end);
		_put(codepoint, memoryProps, device, queue);
		break;
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

} // namespace graphics
