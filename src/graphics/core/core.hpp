#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics::core {

void initializeCore();

void destroyCore() noexcept;

const vk::Instance &instance();

const vk::PhysicalDevice &physicalDevice();

const vk::Device &device();

const vk::Queue &queue();

const vk::CommandPool &commandPool();

} // namespace graphics::core
