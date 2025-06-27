//! 描画処理に関するモジュール
//!
//! 主にレンダーパスやグラフィックスパイプラインを扱う。

#pragma once

#include "../config.hpp"
#include "../error.hpp"

#include <vulkan/vulkan.hpp>

namespace graphics::rendering {

Error initialize(const Config &config, const vk::Device &device);

void terminate(const vk::Device &device);

} // namespace graphics::rendering
