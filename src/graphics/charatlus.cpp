#include "charatlus.hpp"

#include "../config/config.hpp"

#include <vector>

namespace graphics {

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
	std::vector<uint8_t> foo(16 * 24, 255);
	_image.upload(memoryProps, device, queue, 16, 24, 20, 10, foo.data(), true);
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
