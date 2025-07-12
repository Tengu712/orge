#pragma once

#include <vulkan/vulkan.hpp>
#include <yaml-cpp/yaml.h>

namespace config {

struct SubpassConfig {
	const std::vector<vk::AttachmentReference> inputs;
	const std::vector<vk::AttachmentReference> outputs;
	const std::optional<vk::AttachmentReference> depth;

	vk::SubpassDescription get() const {
		return vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setInputAttachments(inputs)
			.setColorAttachments(outputs)
			.setPDepthStencilAttachment(depth ? &(*depth) : nullptr);
	}
};

std::vector<SubpassConfig> parseSubpasses(
	const YAML::Node &node,
	const std::unordered_map<std::string, uint32_t> &attachmentMap,
	std::unordered_map<std::string, uint32_t> &subpassMap
);

} // namespace config
