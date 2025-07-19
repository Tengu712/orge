#pragma once

#include <array>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace config {

enum class Format: uint8_t {
	RenderTarget,
	DepthBuffer,
};

enum class FinalLayout: uint8_t {
	ColorAttachment,
	DepthStencilAttachment,
	PresentSrc,
};

enum class InputLayout: uint8_t {
	DepthStencilReadOnly,
	ShaderReadOnly,
};

enum class DescriptorType: uint8_t {
	CombinedImageSampler,
	UniformBuffer,
	StorageBuffer,
	InputAttachment,
};

enum class ShaderStages: uint8_t {
	Vertex,
	Fragment,
	VertexAndFragment
};

struct AttachmentConfig {
	std::string id;
	Format format;
	bool discard;
	FinalLayout finalLayout;
	std::optional<std::array<float, 4>> colorClearValue;
	std::optional<float> depthClearValue;

	AttachmentConfig(const YAML::Node &node);
};

struct SubpassInputConfig {
	std::string id;
	InputLayout layout;

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
	DescriptorType type;
	uint32_t count;
	ShaderStages stage;

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
	bool depthTest;
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

	std::unordered_map<std::string, uint32_t> attachmentMap;
	std::unordered_map<std::string, uint32_t> subpassMap;

	Config(const YAML::Node &node);
};

Config parse(const char *yaml);

Config parseFromFile(const char *yamlFilePath);

} // namespace config
