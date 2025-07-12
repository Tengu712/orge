#include "pipeline.hpp"

#include "utils.hpp"

#include <iostream>

namespace config {

std::vector<DescriptorSetConfig> parseDescriptorSets(const YAML::Node &node) {
	std::vector<DescriptorSetConfig> descSets;
	for (const auto &n: node["desc-sets"]) {
		std::vector<vk::DescriptorSetLayoutBinding> bindings;
		for (const auto &m: n["bindings"]) {
			const auto type = s(m, "type");
			const auto count = m["count"].as<uint32_t>(1);

			vk::ShaderStageFlags stages{};
			for (const auto &s: a(m, "stages")) {
				const auto t = cs(s, "the element of 'stages'");
				if (t == "vertex") {
					stages |= vk::ShaderStageFlagBits::eVertex;
				} else if (t == "fragment") {
					stages |= vk::ShaderStageFlagBits::eFragment;
				} else {
					throw std::format("config error: pipeline shader stages '{}' is invalid.", t);
				}
			}

			bindings.emplace_back(
				bindings.size(),
				type == "combined-image-sampler" ? vk::DescriptorType::eCombinedImageSampler
				: type == "uniform-buffer" ? vk::DescriptorType::eUniformBuffer
				: type == "storage-buffer" ? vk::DescriptorType::eStorageBuffer
				: type == "input-attachment" ? vk::DescriptorType::eInputAttachment
				: throw std::format("config error: pipeline descriptor type '{}' is invalid.", type),
				count,
				stages,
				nullptr
			);
		}
		descSets.push_back(DescriptorSetConfig {u(n, "count"), std::move(bindings)});
	}
	return descSets;
}

PipelineConfig parsePipeline(
	const YAML::Node &node,
	const std::unordered_map<std::string, uint32_t> &subpassMap,
	std::set<std::string> &ids
) {
	const auto id = s(node, "id");
	if (ids.contains(id)) {
		throw std::format("config error: pipeline id '{}' is duplicated.", id);
	} else {
		ids.insert(id);
	}

	const auto vertexShader = s(node, "vertex-shader");
	const auto fragmentShader = s(node, "fragment-shader");
	const auto descSets = parseDescriptorSets(node);

	std::vector<uint32_t> vertexInputAttributes;
	for (const auto &n: a(node, "vertex-input-attributes")) {
		const auto vertexInputAttribute = cu(n, "the element of 'vertex-input-attributes'");
		if (vertexInputAttribute < 1 || vertexInputAttribute > 4) {
			throw std::format("config error: a vertex input attribute '{}' is invalid.", vertexInputAttribute);
		}
		vertexInputAttributes.push_back(vertexInputAttribute);
	}

	const auto culling = node["culling"].as<bool>(false);

	std::vector<bool> colorBlends;
	for (const auto &n: a(node, "color-blends")) {
		colorBlends.push_back(cb(n, "the element of 'color-blends'"));
	}

	return {
		id,
		vertexShader,
		fragmentShader,
		descSets,
		vertexInputAttributes,
		culling,
		colorBlends,
		at(subpassMap, node, "subpass"),
	};
}

std::vector<PipelineConfig> parsePipelines(
	const YAML::Node &node,
	const std::unordered_map<std::string, uint32_t> &subpassMap
) {
	std::vector<PipelineConfig> pipelines;
	std::set<std::string> ids;
	for (const auto &n: node["pipelines"]) {
		pipelines.push_back(parsePipeline(n, subpassMap, ids));
	}
	return pipelines;
}

} // namespace config
