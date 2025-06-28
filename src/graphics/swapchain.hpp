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
/// 初期化後に呼ばれることを期待する。
Error createFramebuffers(const vk::Device &device, const vk::RenderPass &renderPass, std::vector<vk::Framebuffer> &framebuffers);

/// 利用可能な次のスワップチェインイメージのインデックスを取得する関数
///
/// 初期化後に呼ばれることを期待する。
Error acquireNextImageIndex(const vk::Device &device, uint32_t &index);

/// スワップチェインイメージのサイズを取得する関数
///
/// 初期化後に呼ばれることを期待する。
vk::Extent2D getImageSize();

/// プレゼンテーションを行う関数
///
/// 与えられたセマフォがシグナルされるまでプレゼンテーションを待機する。
/// プレゼンテーションが完了するまでスレッドを待機する。
///
/// 初期化後に呼ばれることを期待する。
Error presentation(const vk::Queue &queue, const vk::Semaphore &semaphore, uint32_t index);

void terminate(const vk::Device &device);

} // namespace graphics::swapchain
