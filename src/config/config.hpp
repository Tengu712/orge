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
	ShareColorAttachment,
	SignedShareColorAttachment,
};

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

struct AttachmentConfig {
	const std::string id;
	const Format format;
	const bool discard;
	const std::optional<std::array<float, 4>> colorClearValue;
	const std::optional<float> depthClearValue;

	AttachmentConfig(const YAML::Node &node);
};

struct SubpassDepthConfig {
	const std::string id;
	const bool readOnly;

	SubpassDepthConfig(const YAML::Node &node);
};

struct SubpassConfig {
	const std::string id;
	const std::vector<std::string> inputs;
	const std::vector<std::string> outputs;
	const std::optional<SubpassDepthConfig> depth;
	const std::vector<std::string> depends;

	SubpassConfig(const YAML::Node &node);
};

struct DescriptorBindingConfig {
	const DescriptorType type;
	const uint32_t count;
	const ShaderStages stage;
	const std::optional<std::string> attachment;

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
	const std::string id;
	const std::string vertexShader;
	const std::string fragmentShader;
	const std::vector<DescriptorSetConfig> descSets;
	const std::vector<uint32_t> vertexInputAttributes;
	const bool culling;
	const bool depthTest;
	const std::vector<bool> colorBlends;
	const std::string subpass;
	const bool textRendering;
	const uint32_t charCount;

	PipelineConfig(const YAML::Node &node);
	PipelineConfig(const YAML::Node &node, const std::string &id);
	PipelineConfig(const YAML::Node &node, uint32_t texCount);
};

struct FontConfig {
	const std::string id;
	const std::string file;
	const uint32_t charSize;
	const uint32_t charAtlusCol;
	const uint32_t charAtlusRow;

	FontConfig(const YAML::Node &node);
};

struct MeshConfig {
	const std::string vertices;
	const std::string indices;

	MeshConfig(const YAML::Node &node);
};

struct Config {
	const std::string title;
	const uint32_t width;
	const uint32_t height;
	const bool fullscreen;
	const bool altReturnToggleFullscreen;
	const uint32_t audioChannelCount;
	const std::unordered_map<std::string, MeshConfig> meshes;
	const std::vector<FontConfig> fonts;
	const std::vector<AttachmentConfig> attachments;
	const std::vector<SubpassConfig> subpasses;
	const std::vector<PipelineConfig> pipelines;

	const std::unordered_map<std::string, uint32_t> attachmentMap;
	const std::unordered_map<std::string, uint32_t> subpassMap;
	const std::unordered_map<std::string, uint32_t> fontMap;
	const std::unordered_map<std::string, uint32_t> assetMap;

	Config(YAML::Node node);
};

void initialize();

const Config &config();

} // namespace config
