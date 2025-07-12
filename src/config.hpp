#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace config {

struct AttachmentConfig {
	std::string id;
	std::string format;
	bool discard;
	std::string finalLayout;
	std::optional<std::array<float, 4>> colorClearValue;
	std::optional<float> depthClearValue;

	AttachmentConfig(const YAML::Node &node);
};

struct SubpassInputConfig {
	std::string id;
	std::string layout;

	SubpassInputConfig(const YAML::Node &node);
};

struct SubpassDepthConfig {
	std::string id;
	bool readOnly;

	SubpassDepthConfig(const YAML::Node &node);
};

struct SubpassConfig {
	std::string id;
	std::vector<SubpassInputConfig> inputs;
	std::vector<std::string> outputs;
	std::optional<SubpassDepthConfig> depth;
	std::vector<std::string> depends;

	SubpassConfig(const YAML::Node &node);
};

struct DescriptorBindingConfig {
	std::string type;
	uint32_t count;
	std::string stages;

	DescriptorBindingConfig(const YAML::Node &node);
};

struct DescriptorSetConfig {
	uint32_t count;
	std::vector<DescriptorBindingConfig> bindings;

	DescriptorSetConfig(const YAML::Node &node);
};

struct PipelineConfig {
	std::string id;
	std::string vertexShader;
	std::string fragmentShader;
	std::vector<DescriptorSetConfig> descSets;
	std::vector<uint32_t> vertexInputAttributes;
	bool culling;
	std::vector<bool> colorBlends;
	std::string subpass;

	PipelineConfig(const YAML::Node &node);
};

struct Config {
	std::string title;
	uint32_t width;
	uint32_t height;
	std::vector<AttachmentConfig> attachments;
	std::vector<SubpassConfig> subpasses;
	std::vector<PipelineConfig> pipelines;

	Config(const YAML::Node &node);
};

Config parse(const char *yaml);

Config parseFromFile(const char *yamlFilePath);

} // namespace config
