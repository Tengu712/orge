#pragma once

#include "../../config/config.hpp"
#include "../core/core.hpp"
#include "../window/swapchain.hpp"

namespace graphics::renderpass {

struct PipelineViewportStateCreateInfoWrapper {
	std::vector<vk::Viewport> viewports;
	std::vector<vk::Rect2D> scissors;
	vk::PipelineViewportStateCreateInfo ci;
};

inline vk::Viewport adjustViewport(uint32_t ow, uint32_t oh, const vk::Extent2D &extent) {
	float o = static_cast<float>(ow)           / static_cast<float>(oh);
	float n = static_cast<float>(extent.width) / static_cast<float>(extent.height);
	if (n > o) {
		const auto h = static_cast<float>(extent.height);
		const auto w = h * o;
		const auto x = (extent.width - w) / 2.0f;
		return vk::Viewport(x, 0.0f, w, h, 0.0f, 1.0f);
	} else {
		const auto w = static_cast<float>(extent.width);
		const auto h = w / o;
		const auto y = (extent.height - h) / 2.0f;
		return vk::Viewport(0.0f, y, w, h, 0.0f, 1.0f);
	}
}

inline PipelineViewportStateCreateInfoWrapper createPipelineViewStateCreateInfo() {
	PipelineViewportStateCreateInfoWrapper result{};
	const auto extent = window::swapchain().getExtent();
	result.viewports.emplace_back(adjustViewport(config::config().width, config::config().height, extent));
	result.scissors.emplace_back(vk::Offset2D(0, 0), extent);
	result.ci = vk::PipelineViewportStateCreateInfo()
		.setViewports(result.viewports)
		.setScissors(result.scissors);
	return result;
}

inline vk::PipelineMultisampleStateCreateInfo createPipelineMultisampleStateCreateInfo() {
	return vk::PipelineMultisampleStateCreateInfo()
		.setRasterizationSamples(vk::SampleCountFlagBits::e1);
}

inline vk::UniqueShaderModule createShaderModule(const unsigned char *data, size_t length) {
	const auto code = std::vector<uint32_t>(
		reinterpret_cast<const uint32_t *>(data),
		reinterpret_cast<const uint32_t *>(data + length)
	);
	const auto ci = vk::ShaderModuleCreateInfo()
		.setCode(code);
	return core::device().createShaderModuleUnique(ci);
}

} // namespace graphics::renderpass
