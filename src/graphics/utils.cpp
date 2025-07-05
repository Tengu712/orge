#include "utils.hpp"

namespace graphics::utils {

uint32_t findMemoryType(const vk::PhysicalDeviceMemoryProperties &memoryProps, uint32_t typeBits, vk::MemoryPropertyFlags mask) {
	uint32_t result = UINT32_MAX;

	for (uint32_t i = 0; i < memoryProps.memoryTypeCount; ++i) {
		if ((typeBits & 1) && ((memoryProps.memoryTypes[i].propertyFlags & mask) == mask)) {
			result = i;
			break;
		}
		typeBits >>= 1;
	}

	if (result == UINT32_MAX) {
		throw "failed to find a correct memory type.";
	}
	return result;
}

} // namespace graphics::utils
