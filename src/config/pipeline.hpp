#pragma once

#include <yaml-cpp/yaml.h>

namespace config {

enum class DescriptorType: uint8_t {
	Texture,
	Sampler,
	UniformBuffer,
	StorageBuffer,
	InputAttachment,
};

enum class ShaderStages: uint8_t {
	Vertex,
	Fragment,
	VertexAndFragment,
};

struct DescriptorBindingConfig {
	const DescriptorType type;
	const uint32_t count;
	const ShaderStages stage;

	DescriptorBindingConfig(const YAML::Node &node);
	DescriptorBindingConfig(DescriptorType type, uint32_t count, ShaderStages stage):
		type(type), count(count), stage(stage)
	{}
};

struct DescriptorSetConfig {
	const uint32_t count;
	const std::vector<DescriptorBindingConfig> bindings;

	DescriptorSetConfig(const YAML::Node &node);
	DescriptorSetConfig(uint32_t count, const std::vector<DescriptorBindingConfig> &&bindings):
		count(count), bindings(bindings)
	{}
};

struct PipelineConfig {
	const std::string vertexShader;
	const std::string fragmentShader;
	const std::vector<DescriptorSetConfig> descSets;
	const std::vector<uint32_t> vertexInputAttributes;
	const bool meshInShader;
	const bool culling;
	const bool depthTest;
	const std::vector<bool> colorBlends;
	const bool textRendering;
	const uint32_t charCount;

	PipelineConfig(const YAML::Node &node);
	PipelineConfig(const YAML::Node &node, uint32_t texCount);
};

std::unordered_map<std::string, PipelineConfig> parsePipelineConfigs(const YAML::Node &node, uint32_t texCount);

} // namespace config
