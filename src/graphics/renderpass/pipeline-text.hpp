#pragma once

#include "pipeline.hpp"

namespace graphics::renderpass {

void createTextRenderingPipeline(
	const vk::RenderPass &renderPass,
	uint32_t subpassIndex,
	std::unordered_map<uint32_t, GraphicsPipeline> &pipelines
);

} // namespace graphics::renderpass
