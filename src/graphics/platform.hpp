//! プラットフォーム特有の処理をまとめたモジュール
//!
//! 主にVulkanをサポートしていないmacOSの所為で必要なモジュール。
//! MoltenVKを有効化するために初期化時に色々と行う必要がある。

#pragma once

#include "../error.hpp"

#include <span>
#include <vulkan/vulkan.hpp>

namespace graphics::platform {

Error initialize();

vk::InstanceCreateFlags getInstanceCreateFlags();

std::span<const char *const> getInstanceExtensions();

std::span<const char *const> getDeviceExtensions();

} // namespace graphics::platform
