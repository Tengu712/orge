#pragma once

#include <yaml-cpp/yaml.h>

namespace config {

enum class ComputeDescriptorType: uint8_t {
	Texture,
	Sampler,
	UniformBuffer,
	StorageBuffer,
	StorageImage,
};

struct ComputeDescriptorBindingConfig {
	const ComputeDescriptorType type;
	const uint32_t count;

	ComputeDescriptorBindingConfig(const YAML::Node &node);
};

struct ComputeDescriptorSetConfig {
	const uint32_t count;
	const std::vector<ComputeDescriptorBindingConfig> bindings;

	ComputeDescriptorSetConfig(const YAML::Node &node);
};

struct ComputePipelineConfig {
	const std::string shader;
	const std::vector<ComputeDescriptorSetConfig> descSets;

	ComputePipelineConfig(const YAML::Node &node);
};

std::unordered_map<std::string, ComputePipelineConfig> parseComputePipelineConfigs(const YAML::Node &node);

} // namespace config
