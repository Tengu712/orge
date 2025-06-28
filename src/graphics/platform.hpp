//! プラットフォームごとに異なるものをまとめたモジュール

#pragma once

#include "../error.hpp"

#include <array>
#include <vulkan/vulkan.hpp>

namespace graphics::platform {

#ifdef __APPLE__
constexpr vk::InstanceCreateFlags getInstanceCreateFlags() {
	return vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
}

constexpr std::array<const char *const, 1> getInstanceExtensions() {
	return {"VK_KHR_portability_enumeration"};
}

constexpr std::array<const char *const, 1> getDeviceExtensions() {
	return {"VK_KHR_portability_subset"};
}

constexpr vk::Format getRenderTargetPixelFormat() {
	return vk::Format::eB8G8R8A8Unorm;
}

constexpr vk::ColorSpaceKHR getRenderTargetColorSpace() {
	return vk::ColorSpaceKHR::eSrgbNonlinear;
}
#else
constexpr vk::InstanceCreateFlags getInstanceCreateFlags() {
	return vk::InstanceCreateFlags();
}

constexpr std::array<const char *const, 0> getInstanceExtensions() {
	return {};
}

constexpr std::array<const char *const, 0> getDeviceExtensions() {
	return {};
}

constexpr vk::Format getRenderTargetPixelFormat() {
	return vk::Format::eR8G8B8A8Srgb;
}

constexpr vk::ColorSpaceKHR getRenderTargetColorSpace() {
	return vk::ColorSpaceKHR::eSrgbNonlinear;
}
#endif

} // namespace graphics::platform
