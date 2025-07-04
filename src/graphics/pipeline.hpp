#pragma once

#include "../config.hpp"
#include "../error.hpp"

#include <vulkan/vulkan.hpp>

namespace graphics::pipeline {

Error initialize(const Config &config_, const vk::Device &device, const vk::RenderPass &renderPass);

void terminate(const vk::Device &device);

} // namespace graphics::pipeline
