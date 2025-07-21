#pragma once

#include "../../../config/config.hpp"

#include <vulkan/vulkan.hpp>

namespace graphics::rendering::swapchain {

void terminate(const vk::Instance &instance, const vk::Device &device);

std::span<const char *const> getInstanceExtensions();

std::span<const char *const> getDeviceExtensions();

void initialize(
	const config::Config &config,
	const vk::Instance &instance,
	const vk::PhysicalDevice &physicalDevice,
	const vk::Device &device
);

const std::vector<vk::Image> &getImages();

vk::Extent2D getImageSize();

/// 利用可能な次のスワップチェインイメージのインデックスを取得する関数
///
/// イメージの取得が完了したら与えられたセマフォをシグナルする。
uint32_t acquireNextImageIndex(const vk::Device &device, const vk::Semaphore &semaphore);

/// プレゼンテーションを行う関数
///
/// 与えられたセマフォがシグナルされるまでプレゼンテーションを待機する。
/// プレゼンテーションが完了するまでスレッドを待機する。
void presentation(const vk::Queue &queue, const vk::Semaphore &semaphore, uint32_t index);

void toggleFullscreen();

} // namespace graphics::rendering::swapchain
