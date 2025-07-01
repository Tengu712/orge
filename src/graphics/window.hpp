//! ウィンドウに関するモジュール

#pragma once

#include "../error.hpp"

#include <optional>
#include <span>
#include <string>
#include <vulkan/vulkan.hpp>

namespace graphics::window {

Error createWindow(const std::string &title, int width, int height);

/// Vulkanインスタンスに必要なextensionsを取得する関数
///
/// 既にウィンドウが生成されていることを期待する。
std::span<const char *const> getInstanceExtensions();

/// サーフェスを作成する関数
///
/// 既にウィンドウが生成されていることを期待する。
Error createSurface(const vk::Instance &instance, vk::SurfaceKHR &surface);

/// ウィンドウイベントをすべて処理する関数
///
/// ウィンドウが閉じられたときにfalseを返す。
///
/// 既にウィンドウが生成されていることを期待する。
bool pollEvents();

void terminate();

} // namespace graphics::window
