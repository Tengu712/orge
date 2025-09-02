#include "pipeline.hpp"

#include "utils.hpp"

namespace config {

inline DescriptorType parseDescriptorType(const std::string& s) {
	return s == "texture"
		? DescriptorType::Texture
		: s == "sampler"
		? DescriptorType::Sampler
		: s == "uniform-buffer"
		? DescriptorType::UniformBuffer
		: s == "storage-buffer"
		? DescriptorType::StorageBuffer
		: s == "input-attachment"
		? DescriptorType::InputAttachment
		: throw std::format("config error: type '{}' is invalid.", s);
}

inline ShaderStages parseShaderStages(const std::string& s) {
	return s == "vertex"
		? ShaderStages::Vertex
		: s == "fragment"
		? ShaderStages::Fragment
		: s == "vertex-and-fragment"
		? ShaderStages::VertexAndFragment
		: throw std::format("config error: stages '{}' is invalid.", s);
}

DescriptorBindingConfig::DescriptorBindingConfig(const YAML::Node &node):
	type(parseDescriptorType(s(node, "type"))),
	count(u(node, "count", 1)),
	stage(parseShaderStages(s(node, "stage")))
{
	checkUnexpectedKeys(node, {"type", "count", "stage"});
}

DescriptorSetConfig::DescriptorSetConfig(const YAML::Node &node):
	count(u(node, "count")),
	bindings(parseConfigs<DescriptorBindingConfig>(node, "bindings"))
{
	checkUnexpectedKeys(node, {"count", "bindings"});
}

PipelineConfig::PipelineConfig(const YAML::Node &node):
	vertexShader(s(node, "vertex-shader")),
	fragmentShader(s(node, "fragment-shader")),
	descSets(parseConfigs<DescriptorSetConfig>(node, "desc-sets")),
	vertexInputAttributes(us(node, "vertex-input-attributes")),
	culling(b(node, "culling", false)),
	depthTest(b(node, "depth-test", false)),
	colorBlends(bs(node, "color-blends")),
	textRendering(false),
	charCount(0)
{
	checkUnexpectedKeys(
		node,
		{
			"id", "vertex-shader", "fragment-shader", "desc-sets", "vertex-input-attributes",
			"culling", "depth-test", "color-blends"
		}
	);

	for (const auto &n: vertexInputAttributes) {
		validateValue(n, {1, 2, 3, 4}, "vertex input attribute");
	}
}

std::vector<DescriptorSetConfig> createTextRenderingPipelineDescSets(uint32_t texCount) {
	std::vector<DescriptorSetConfig> descSets;

	std::vector<DescriptorBindingConfig> bindings1;
	bindings1.emplace_back(DescriptorType::StorageBuffer, 1, ShaderStages::Vertex);
	descSets.emplace_back(1, std::move(bindings1));

	std::vector<DescriptorBindingConfig> bindings2;
	bindings2.emplace_back(DescriptorType::Texture, texCount, ShaderStages::Fragment);
	bindings2.emplace_back(DescriptorType::Sampler, 1, ShaderStages::Fragment);
	descSets.emplace_back(1, std::move(bindings2));

	return descSets;
}

PipelineConfig::PipelineConfig(const YAML::Node &node, uint32_t texCount):
	vertexShader(""),
	fragmentShader(""),
	descSets(createTextRenderingPipelineDescSets(texCount)),
	vertexInputAttributes(),
	culling(false),
	depthTest(false),
	colorBlends{true},
	textRendering(true),
	charCount(u(node, "char-count"))
{
	checkUnexpectedKeys(node, {"id", "text-rendering", "subpass", "render-passes", "char-count"});
}

std::unordered_map<std::string, PipelineConfig> parsePipelineConfigs(const YAML::Node &node, uint32_t texCount) {
	std::unordered_map<std::string, PipelineConfig> pipelines;
	for (const auto &n: node["pipelines"]) {
		const auto id = s(n, "id");
		if (pipelines.contains(id)) {
			throw std::format("config error: pipeline '{}' duplicated.", id);
		}
		if (b(n, "text-rendering", false)) {
			pipelines.emplace(id, PipelineConfig(n, texCount));
		} else {
			pipelines.emplace(id, PipelineConfig(n));
		}
	}
	return pipelines;
}

} // namespace config
