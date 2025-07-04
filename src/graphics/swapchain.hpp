//! スワップチェインに関するモジュール

#pragma once

#include "../error.hpp"

#include <span>
#include <vulkan/vulkan.hpp>

namespace graphics::swapchain {

/// Vulkanデバイスに必要なextensionsを取得する関数
std::span<const char *const> getDeviceExtensions();

void initialize(const vk::Instance &instance, const vk::PhysicalDevice &physicalDevice, const vk::Device &device);

/// フレームバッファを作成する関数
///
/// 初期化後に呼ばれることを期待する。
std::vector<vk::Framebuffer> createFramebuffers(const vk::Device &device, const vk::RenderPass &renderPass);

/// 利用可能な次のスワップチェインイメージのインデックスを取得する関数
///
/// イメージの取得が完了したら与えられたセマフォをシグナルする。
///
/// 初期化後に呼ばれることを期待する。
uint32_t acquireNextImageIndex(const vk::Device &device, const vk::Semaphore &semaphore);

/// スワップチェインイメージの個数を取得する関数
///
/// 初期化後に呼ばれることを期待する。
uint32_t getImageCount();

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
void presentation(const vk::Queue &queue, const vk::Semaphore &semaphore, uint32_t index);

void terminate(const vk::Instance &instance, const vk::Device &device);

} // namespace graphics::swapchain
