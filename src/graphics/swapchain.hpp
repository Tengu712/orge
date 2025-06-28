//! スワップチェインに関するモジュール

#pragma once

#include "../error.hpp"

#include <span>
#include <vulkan/vulkan.hpp>

namespace graphics::swapchain {

/// Vulkanデバイスに必要なextensionsを取得する関数
std::span<const char *const> getDeviceExtensions();

Error initialize(const vk::PhysicalDevice &physicalDevice, const vk::Device &device, const vk::SurfaceKHR &surface);

/// フレームバッファを作成する関数
///
/// 失敗時は空のベクタが返される。
///
/// 初期化後に呼ばれることを期待する。
std::vector<vk::Framebuffer> createFrameBuffers(const vk::Device &device, const vk::RenderPass &renderPass);

void terminate(const vk::Device &device);

} // namespace graphics::swapchain
