#include "compute.hpp"

#include "utils.hpp"

namespace config {

inline ComputeDescriptorType parseComputeDescriptorType(const std::string& s) {
	return s == "texture"
		? ComputeDescriptorType::Texture
		: s == "sampler"
		? ComputeDescriptorType::Sampler
		: s == "uniform-buffer"
		? ComputeDescriptorType::UniformBuffer
		: s == "storage-buffer"
		? ComputeDescriptorType::StorageBuffer
		: s == "storage-image"
		? ComputeDescriptorType::StorageImage
		: throw std::format("config error: type '{}' is invalid.", s);
}

ComputeDescriptorBindingConfig::ComputeDescriptorBindingConfig(const YAML::Node &node):
	type(parseComputeDescriptorType(s(node, "type"))),
	count(u(node, "count", 1))
{
	checkUnexpectedKeys(node, {"type", "count"});
}

ComputeDescriptorSetConfig::ComputeDescriptorSetConfig(const YAML::Node &node):
	count(u(node, "count")),
	bindings(parseConfigs<ComputeDescriptorBindingConfig>(node, "bindings"))
{
	checkUnexpectedKeys(node, {"count", "bindings"});
}

ComputePipelineConfig::ComputePipelineConfig(const YAML::Node &node):
	shader(s(node, "shader")),
	descSets(parseConfigs<ComputeDescriptorSetConfig>(node, "desc-sets"))
{
	checkUnexpectedKeys(node, {"id", "shader", "desc-sets"});
}

std::unordered_map<std::string, ComputePipelineConfig> parseComputePipelineConfigs(const YAML::Node &node) {
	std::unordered_map<std::string, ComputePipelineConfig> pipelines;
	for (const auto &n: node["compute-pipelines"]) {
		const auto id = s(n, "id");
		if (pipelines.contains(id)) {
			throw std::format("config error: compute pipeline '{}' duplicated.", id);
		}
		pipelines.emplace(id, n);
	}
	return pipelines;
}

} // namespace config
