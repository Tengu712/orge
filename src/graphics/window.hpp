//! ウィンドウに関するモジュール

#pragma once

#include <span>
#include <string>
#include <vulkan/vulkan.hpp>

namespace graphics::window {

void initialize(const std::string &title, int width, int height);

/// Vulkanインスタンスに必要なextensionsを取得する関数
///
/// 既にウィンドウが生成されていることを期待する。
std::span<const char *const> getInstanceExtensions();

/// サーフェスを作成する関数
///
/// 既にウィンドウが生成されていることを期待する。
vk::SurfaceKHR createSurface(const vk::Instance &instance);

/// ウィンドウイベントをすべて処理する関数
///
/// ウィンドウが閉じられたときにfalseを返す。
///
/// 既にウィンドウが生成されていることを期待する。
bool pollEvents();

void terminate();

} // namespace graphics::window
