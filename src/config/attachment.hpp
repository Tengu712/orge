#pragma once

#include <vulkan/vulkan.hpp>
#include <yaml-cpp/yaml.h>

namespace config {

std::vector<vk::AttachmentDescription> parseAttachments(
	const YAML::Node &node,
	std::unordered_map<std::string, uint32_t> &attachmentMap
);

std::vector<vk::ClearValue> parseClearValues(const YAML::Node &node);

} // namespace config
