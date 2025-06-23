//! ウィンドウに関するモジュール

#pragma once

#include "../error.hpp"

#include <optional>
#include <span>
#include <vulkan/vulkan.hpp>

namespace graphics::window {

Error createWindow(const char *title, int width, int height);

/// Vulkanインスタンスに必要なextensionsを取得する関数
///
/// 既にウィンドウが生成されていることを期待する。
std::span<const char *const> getExtensions();

/// サーフェスを作成する関数
///
/// 既にウィンドウが生成されていることを期待する。
std::optional<vk::SurfaceKHR> createSurface(const vk::Instance &instance);

void terminate();

} // namespace graphics::window
