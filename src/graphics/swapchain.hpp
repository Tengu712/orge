//! スワップチェインに関するモジュール

#pragma once

#include "../error.hpp"

#include <span>
#include <vulkan/vulkan.hpp>

namespace graphics::swapchain {

/// Vulkanデバイスに必要なextensionsを取得する関数
std::span<const char *const> getExtensions();

Error initialize(const vk::PhysicalDevice &physicalDevice, const vk::Device &device, const vk::SurfaceKHR &surface);

void terminate();

} // namespace graphics::swapchain
