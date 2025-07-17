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

constexpr vk::Format platformRenderTargetPixelFormat() {
	return vk::Format::eB8G8R8A8Unorm;
}

constexpr vk::ColorSpaceKHR platformRenderTargetColorSpace() {
	return vk::ColorSpaceKHR::eSrgbNonlinear;
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

constexpr vk::Format platformRenderTargetPixelFormat() {
	return vk::Format::eR8G8B8A8Srgb;
}

constexpr vk::ColorSpaceKHR platformRenderTargetColorSpace() {
	return vk::ColorSpaceKHR::eSrgbNonlinear;
}
#endif

} // namespace graphics
