#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics::resource {

void initializeDescriptorPool();

void destroyDescriptorPool() noexcept;

const vk::DescriptorPool &descpool();

} // namespace graphics::resource
