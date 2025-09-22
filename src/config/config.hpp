#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <orge/orge.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace config {

struct FontConfig {
	const std::string file;
	const uint32_t charSize;
	const uint32_t charAtlusCol;
	const uint32_t charAtlusRow;

	FontConfig(const char *file, uint32_t size, uint32_t col, uint32_t row):
		file(file), charSize(size), charAtlusCol(col), charAtlusRow(row)
	{}
};

enum class Format {
	RenderTarget,
	DepthBuffer,
	ShareColorAttachment,
	SignedShareColorAttachment,
};

using ColorClearValue = std::array<float, 4>;
using DepthClearValue = float;
using ClearValueConfig = std::variant<ColorClearValue, DepthClearValue>;

struct AttachmentConfig {
	const Format format;
	const bool discard;
	const ClearValueConfig clearValue;

	AttachmentConfig(Format format, bool discard, ClearValueConfig clearValue):
		format(format), discard(discard), clearValue(clearValue)
	{}
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

struct DescriptorBindingConfig {
	const DescriptorType type;
	const uint32_t count;
	const ShaderStages stage;

	DescriptorBindingConfig(DescriptorType type, uint32_t count, ShaderStages stage):
		type(type), count(count), stage(stage)
	{}
};

struct DescriptorSetConfig {
	const uint32_t count;
	const std::vector<DescriptorBindingConfig> bindings;

	DescriptorSetConfig(uint32_t count, std::vector<DescriptorBindingConfig> &&bindings):
		count(count), bindings(std::move(bindings))
	{}
};

struct GraphicsPipelineConfig {
	const std::string vertexShader;
	const std::string fragmentShader;
	const std::vector<DescriptorSetConfig> descSets;
	const std::vector<uint32_t> vertexInputAttributes;
	const bool meshInShader;
	const bool culling;
	const bool depthTest;
	const std::vector<bool> colorBlends;

	GraphicsPipelineConfig(
		const char *vertexShader,
		const char *fragmentShader,
		std::vector<DescriptorSetConfig> &&descSets,
		std::vector<uint32_t> &&vertexInputAttributes,
		bool meshInShader,
		bool culling,
		bool depthTest,
		std::vector<bool> &&colorBlends
	):
		vertexShader(vertexShader),
		fragmentShader(fragmentShader),
		descSets(std::move(descSets)),
		vertexInputAttributes(std::move(vertexInputAttributes)),
		meshInShader(meshInShader),
		culling(culling),
		depthTest(depthTest),
		colorBlends(std::move(colorBlends))
	{}
};

struct SubpassDepthConfig {
	const std::string id;
	const bool readOnly;

	SubpassDepthConfig(const char *id, bool readOnly): id(id), readOnly(readOnly) {}
};

struct SubpassConfig {
	const std::string id;
	const std::unordered_set<std::string> inputs;
	const std::unordered_set<std::string> outputs;
	const std::optional<SubpassDepthConfig> depth;
	const std::unordered_set<std::string> depends;
	const std::unordered_set<std::string> pipelines;

	SubpassConfig(
		const char *id,
		std::unordered_set<std::string> &&inputs,
		std::unordered_set<std::string> &&outputs,
		std::optional<SubpassDepthConfig> &&depth,
		std::unordered_set<std::string> &&depends,
		std::unordered_set<std::string> &&pipelines
	):
		id(id),
		inputs(std::move(inputs)),
		outputs(std::move(outputs)),
		depth(std::move(depth)),
		depends(std::move(depends)),
		pipelines(std::move(pipelines))
	{}
};

struct RenderPassConfig {
	const std::vector<SubpassConfig> subpasses;

	const std::vector<std::string> attachments;
	const std::unordered_map<std::string, uint32_t> attachmentMap;
	const std::unordered_map<std::string, uint32_t> subpassMap;

	RenderPassConfig(
		std::vector<SubpassConfig> &&subpasses,
		std::vector<std::string> &&attachments,
		std::unordered_map<std::string, uint32_t> &&attachmentMap,
		std::unordered_map<std::string, uint32_t> &&subpassMap
	):
		subpasses(std::move(subpasses)),
		attachments(std::move(attachments)),
		attachmentMap(std::move(attachmentMap)),
		subpassMap(std::move(subpassMap))
	{}
};

enum class ComputeDescriptorType {
	Texture,
	Sampler,
	UniformBuffer,
	StorageBuffer,
	StorageImage,
};

struct ComputeDescriptorBindingConfig {
	const ComputeDescriptorType type;
	const uint32_t count;

	ComputeDescriptorBindingConfig(ComputeDescriptorType type, uint32_t count):
		type(type), count(count)
	{}
};

struct ComputeDescriptorSetConfig {
	const uint32_t count;
	const std::vector<ComputeDescriptorBindingConfig> bindings;

	ComputeDescriptorSetConfig(uint32_t count, std::vector<ComputeDescriptorBindingConfig> &&bindings):
		count(count), bindings(std::move(bindings))
	{}
};

struct ComputePipelineConfig {
	const std::string shader;
	const std::vector<ComputeDescriptorSetConfig> descSets;

	ComputePipelineConfig(const char *shader, std::vector<ComputeDescriptorSetConfig> &&descSets):
		shader(shader), descSets(std::move(descSets))
	{}
};

struct Config {
	const std::string title;
	const uint32_t width;
	const uint32_t height;
	const bool fullscreen;
	const bool disableFullscreenShortcut;
	const bool disableVsync;
	const uint32_t audioChannelCount;
	const uint32_t charCount;
	const std::unordered_map<std::string, FontConfig> fonts;
	const std::unordered_map<std::string, AttachmentConfig> attachments;
	const std::unordered_map<std::string, GraphicsPipelineConfig> pipelines;
	const std::unordered_map<std::string, RenderPassConfig> renderPasses;
	const std::unordered_map<std::string, ComputePipelineConfig> computePipelines;

	const std::unordered_map<std::string, uint32_t> assetMap;
	const std::unordered_map<std::string, uint32_t> fontMap;

	Config(
		const char *title,
		uint32_t width,
		uint32_t height,
		bool fullscreen,
		bool disableFullscreenShortcut,
		bool disableVsync,
		uint32_t audioChannelCount,
		uint32_t charCount,
		std::unordered_map<std::string, FontConfig> &&fonts,
		std::unordered_map<std::string, AttachmentConfig> &&attachments,
		std::unordered_map<std::string, GraphicsPipelineConfig> &&pipelines,
		std::unordered_map<std::string, RenderPassConfig> &&renderPasses,
		std::unordered_map<std::string, ComputePipelineConfig> &&computePipelines,
		std::unordered_map<std::string, uint32_t> &&assetMap,
		std::unordered_map<std::string, uint32_t> &&fontMap
	):
		title(title),
		width(width),
		height(height),
		fullscreen(fullscreen),
		disableFullscreenShortcut(disableFullscreenShortcut),
		disableVsync(disableVsync),
		audioChannelCount(audioChannelCount),
		charCount(charCount),
		fonts(std::move(fonts)),
		attachments(std::move(attachments)),
		pipelines(std::move(pipelines)),
		renderPasses(std::move(renderPasses)),
		computePipelines(std::move(computePipelines)),
		assetMap(std::move(assetMap)),
		fontMap(std::move(fontMap))
	{}
};

void initialize(OrgeInitializeParam *param);

const Config &config();

} // namespace config
