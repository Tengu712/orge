#pragma once

#include <vulkan/vulkan.hpp>
#include <yaml-cpp/yaml.h>

namespace config {

std::vector<vk::SubpassDependency> parseDependencies(
	const YAML::Node &node,
	const std::unordered_map<std::string, uint32_t> &subpassMap
);

} // namespace config
