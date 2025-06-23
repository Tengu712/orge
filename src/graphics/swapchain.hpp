//! スワップチェインに関するモジュール

#pragma once

#include "../error.hpp"

#include <vulkan/vulkan.hpp>

namespace graphics::swapchain {

Error initialize(const vk::SurfaceKHR &surface);

void terminate();

} // namespace graphics::swapchain
