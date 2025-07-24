#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics {

#ifdef __APPLE__
constexpr vk::InstanceCreateFlags platformInstanceCreateFlags() {
	return vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
}

constexpr std::array<const char *const, 1> platformInstanceExtensions() {
	return {"VK_KHR_portability_enumeration"};
}

constexpr std::array<const char *const, 1> platformDeviceExtensions() {
	return {"VK_KHR_portability_subset"};
}
#else
constexpr vk::InstanceCreateFlags platformInstanceCreateFlags() {
	return vk::InstanceCreateFlags();
}

constexpr std::array<const char *const, 0> platformInstanceExtensions() {
	return {};
}

constexpr std::array<const char *const, 0> platformDeviceExtensions() {
	return {};
}
#endif

} // namespace graphics
