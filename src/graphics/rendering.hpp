//! 描画処理に関するモジュール
//!
//! 主にレンダーパスやグラフィックスパイプラインを扱う。

#pragma once

#include "../config.hpp"
#include "../error.hpp"

#include <vulkan/vulkan.hpp>

namespace graphics::rendering {

void initialize(const Config &config, const vk::Device &device, const vk::CommandPool &commandPool);

void render(const vk::Device &device, const vk::Queue &queue);

void terminate(const vk::Device &device);

} // namespace graphics::rendering
