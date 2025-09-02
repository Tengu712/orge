#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics::resource {

void destroyAllSamplers() noexcept;

void addSampler(const std::string &id, bool linearMagFilter, bool linearMinFilter, bool repeat);

void destroySampler(const std::string &id) noexcept;

const vk::Sampler &getSampler(const std::string &id);

} // namespace graphics::resource
