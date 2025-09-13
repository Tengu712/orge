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
	vertexInputAttributes(us(node, "vertex-input-attributes", std::vector<uint32_t>{})),
	meshInShader(b(node, "mesh-in-shader", false)),
	culling(b(node, "culling", false)),
	depthTest(b(node, "depth-test", false)),
	colorBlends(bs(node, "color-blends"))
{
	checkUnexpectedKeys(
		node,
		{
			"id", "vertex-shader", "fragment-shader", "desc-sets", "vertex-input-attributes",
			"mesh-in-shader", "culling", "depth-test", "color-blends"
		}
	);

	for (const auto &n: vertexInputAttributes) {
		validateValue(n, {1, 2, 3, 4}, "vertex input attribute");
	}
}

std::unordered_map<std::string, PipelineConfig> parsePipelineConfigs(const YAML::Node &node) {
	std::unordered_map<std::string, PipelineConfig> pipelines;
	for (const auto &n: node["pipelines"]) {
		const auto id = s(n, "id");
		if (pipelines.contains(id)) {
			throw std::format("config error: pipeline '{}' duplicated.", id);
		}
		pipelines.emplace(id, n);
	}
	return pipelines;
}

} // namespace config
