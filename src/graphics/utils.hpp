#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics::utils {

uint32_t findMemoryType(const vk::PhysicalDeviceMemoryProperties &memoryProps, uint32_t typeBits, vk::MemoryPropertyFlags mask);

} // namespace graphics::utils
