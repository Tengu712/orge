#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics::rendering::pipeline::sampler {

void terminate(const vk::Device &device);

void create(const vk::Device &device, const char *id, int linearMagFilter, int linearMinFilter, int repeat);

void destroy(const vk::Device &device, const char *id);

const vk::Sampler &get(const char *id);

} // namespace graphics::rendering::pipeline::sampler
