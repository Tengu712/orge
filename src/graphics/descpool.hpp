#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics {

vk::DescriptorPool createDescriptorPool(const vk::Device &device);

} // namespace graphics
