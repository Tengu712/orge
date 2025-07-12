#pragma once

#include <span>
#include <string>
#include <vulkan/vulkan.hpp>

namespace graphics::window {

void initialize(const std::string &title, int width, int height);

std::span<const char *const> getInstanceExtensions();

vk::SurfaceKHR createSurface(const vk::Instance &instance);

void terminate();

} // namespace graphics::window
