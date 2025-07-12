#pragma once

#include "../config.hpp"
#include "../error.hpp"

#include <vulkan/vulkan.hpp>

namespace graphics::rendering {

void initialize(const config::Config &config, const vk::Device &device, const vk::CommandPool &commandPool);

void beginRender(const vk::Device &device);

void endRender(const vk::Queue &queue);

void terminate(const vk::Device &device);

} // namespace graphics::rendering
