#pragma once

#include "../config/config.hpp"

#include <vulkan/vulkan.hpp>

namespace graphics {

vk::DescriptorPool createDescriptorPool(const config::Config &config, const vk::Device &device);

} // namespace graphics
