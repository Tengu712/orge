#pragma once

#include "../config.hpp"
#include "../error.hpp"

#include <vulkan/vulkan.hpp>

namespace graphics::pipeline {

Error initialize(const vk::Device &device);

void terminate();

} // namespace graphics::pipeline
