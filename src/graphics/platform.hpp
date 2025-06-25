//! プラットフォーム特有の処理をまとめたモジュール
//!
//! 主にVulkanをサポートしていないmacOSの所為で必要なモジュール。
//! MoltenVKを有効化するために初期化時に色々と行う必要がある。

#pragma once

#include "../error.hpp"

#include <array>
#include <vulkan/vulkan.hpp>

namespace graphics::platform {

Error initialize();

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
