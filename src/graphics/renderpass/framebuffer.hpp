#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics::renderpass {

std::vector<vk::UniqueFramebuffer> createFramebuffers(
	const vk::RenderPass &renderPass,
	const std::string &renderPassId
);

} // namespace graphics::renderpass
