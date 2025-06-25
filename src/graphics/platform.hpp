//! プラットフォーム特有の処理をまとめたモジュール
//!
//! 主にVulkanをサポートしていないmacOSの所為で必要なモジュール。
//! MoltenVKを有効化するために初期化時に色々と行う必要がある。

#pragma once

#include "../error.hpp"

#include <span>
#include <vulkan/vulkan.hpp>

namespace graphics::platform {

#ifdef __APPLE__
constexpr vk::Format getRenderTargetPixelFormat() {
	return vk::Format::eB8G8R8A8Unorm;
}

constexpr vk::ColorSpaceKHR getRenderTargetColorSpace() {
	return vk::ColorSpaceKHR::eSrgbNonlinear;
}
#else
constexpr vk::Format getRenderTargetPixelFormat() {
	return vk::Format::eR8G8B8A8Srgb;
}

constexpr vk::ColorSpaceKHR getRenderTargetColorSpace() {
	return vk::ColorSpaceKHR::eSrgbNonlinear;
}
#endif

Error initialize();

vk::InstanceCreateFlags getInstanceCreateFlags();

std::span<const char *const> getInstanceExtensions();

std::span<const char *const> getDeviceExtensions();

} // namespace graphics::platform
