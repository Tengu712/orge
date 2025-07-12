#pragma once

#include <vulkan/vulkan.hpp>
#include <yaml-cpp/yaml.h>

namespace config {

struct DescriptorSetConfig {
	uint32_t count;
	std::vector<vk::DescriptorSetLayoutBinding> bindings;
};

struct PipelineConfig {
	std::string id;
	std::string vertexShader;
	std::string fragmentShader;
	std::vector<DescriptorSetConfig> descSets;
	std::vector<uint32_t> vertexInputAttributes;
	bool culling;
	std::vector<bool> colorBlends;
	uint32_t subpass;
};

std::vector<PipelineConfig> parsePipelines(
	const YAML::Node &node,
	const std::unordered_map<std::string, uint32_t> &subpassMap
);

} // namespace config
