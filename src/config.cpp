#include "config.hpp"

#include "graphics/platform.hpp"

#include <set>
#include <unordered_map>
#include <yaml-cpp/yaml.h>

Attachment parseAttachment(const YAML::Node &node, std::unordered_map<std::string, uint32_t> &attachmentMap) {
	const auto id = node["id"].as<std::string>();
	if (!attachmentMap.emplace(id, static_cast<uint32_t>(attachmentMap.size())).second) {
		throw;
	}
	const auto format = node["format"].as<std::string>();
	const auto discard = node["discard"].as<bool>(false);
	const auto finalLayout = node["final-layout"].as<std::string>();
	const auto clearValue = node["clear-value"];
	return {
		format == "render-target" ? graphics::platform::getRenderTargetPixelFormat()
		: throw,
		discard,
		finalLayout == "general" ? vk::ImageLayout::eGeneral
		: finalLayout == "color-attachment" ? vk::ImageLayout::eColorAttachmentOptimal
		: finalLayout == "depth-stencil-attachment" ? vk::ImageLayout::eDepthStencilAttachmentOptimal
		: finalLayout == "transfer-src" ? vk::ImageLayout::eTransferSrcOptimal
		: finalLayout == "present-src" ? vk::ImageLayout::ePresentSrcKHR
		: throw,
		clearValue.IsSequence() && clearValue.size() == 4
		? static_cast<vk::ClearValue>(vk::ClearColorValue(clearValue[0].as<float>(), clearValue[1].as<float>(), clearValue[2].as<float>(), clearValue[3].as<float>()))
		: clearValue.IsScalar()
		? static_cast<vk::ClearValue>(vk::ClearDepthStencilValue(clearValue.as<float>(), 0))
		: throw,
	};
}

Subpass parseSubpass(const YAML::Node &node, const std::unordered_map<std::string, uint32_t> &attachmentMap, std::unordered_map<std::string, uint32_t> &subpassMap) {
	const auto id = node["id"].as<std::string>();
	if (!subpassMap.emplace(id, static_cast<uint32_t>(subpassMap.size())).second) {
		throw;
	}

	std::vector<vk::AttachmentReference> inputs;
	if (node["inputs"]) {
		for (const auto &n : node["inputs"]) {
			const auto id = n["id"].as<std::string>();
			const auto layout = n["layout"].as<std::string>();
			inputs.emplace_back(
				attachmentMap.at(id),
				layout == "general" ? vk::ImageLayout::eGeneral
				: layout == "depth-stencil-read-only" ? vk::ImageLayout::eDepthStencilReadOnlyOptimal
				: layout == "shader-read-only" ? vk::ImageLayout::eShaderReadOnlyOptimal
				: throw
			);
		}
	}

	std::vector<vk::AttachmentReference> outputs;
	for (const auto &n : node["outputs"]) {
		const auto id = n["id"].as<std::string>();
		const auto layout = n["layout"].as<std::string>();
		outputs.emplace_back(
			attachmentMap.at(id),
			layout == "general" ? vk::ImageLayout::eGeneral
			: layout == "color-attachment" ? vk::ImageLayout::eColorAttachmentOptimal
			: throw
		);
	}

	std::optional<vk::AttachmentReference> depth;
	if (node["depth"]) {
		const auto &n = node["depth"];
		const auto id = n["id"].as<std::string>();
		const auto layout = n["layout"].as<std::string>();
		depth.emplace(
			attachmentMap.at(id),
			layout == "general" ? vk::ImageLayout::eGeneral
			: layout == "depth-stencil-attachment" ? vk::ImageLayout::eDepthStencilAttachmentOptimal
			: layout == "depth-stencil-read-only" ? vk::ImageLayout::eDepthStencilReadOnlyOptimal
			: throw
		);
	}

	return {
		inputs,
		outputs,
		depth,
	};
}

Pipeline parsePipeline(const YAML::Node &node, const std::unordered_map<std::string, uint32_t> &subpassMap) {
	const auto id = node["id"].as<std::string>();
	const auto vertexShader = node["vertexShader"].as<std::string>();
	const auto fragmentShader = node["fragmentShader"].as<std::string>();
	std::vector<std::vector<vk::DescriptorSetLayoutBinding>> descSets;
	for (const auto &m: node["descSets"]) {
		std::vector<vk::DescriptorSetLayoutBinding> descs;
		for (const auto &n: node["descs"]) {
			const auto type = n["type"].as<std::string>();
			const auto count = n["count"].as<uint32_t>(1);
			const auto stages = n["stages"].as<std::string>();
			descs.emplace_back(
				descs.size(),
				type == "sampler" ? vk::DescriptorType::eSampler
				: type == "combined-image-sampler" ? vk::DescriptorType::eCombinedImageSampler
				: type == "sampled-image" ? vk::DescriptorType::eSampledImage
				: type == "storage-image" ? vk::DescriptorType::eStorageImage
				: type == "uniform-texel-buffer" ? vk::DescriptorType::eUniformTexelBuffer
				: type == "storage-texel-buffer" ? vk::DescriptorType::eStorageTexelBuffer
				: type == "uniform-buffer" ? vk::DescriptorType::eUniformBuffer
				: type == "storage-buffer" ? vk::DescriptorType::eStorageBuffer
				: type == "uniform-buffer-dynamic" ? vk::DescriptorType::eUniformBufferDynamic
				: type == "storage-buffer-dynamic" ? vk::DescriptorType::eStorageBufferDynamic
				: type == "input-attachment" ? vk::DescriptorType::eInputAttachment
				: throw,
				count,
				stages == "vertex" ? vk::ShaderStageFlagBits::eVertex
				: stages == "fragment" ? vk::ShaderStageFlagBits::eFragment
				: throw,
				nullptr
			);
		}
		descSets.push_back(std::move(descs));
	}
	std::vector<uint32_t> vertexInputAttributes;
	for (const auto &n: node["vertexInputAttributes"]) {
		const auto vertexInputAttribute = n.as<uint32_t>();
		if (vertexInputAttribute < 1 || vertexInputAttribute > 4) {
			throw;
		}
		vertexInputAttributes.push_back(vertexInputAttribute);
	}
	const auto culling = node["culling"].as<bool>(false);
	std::vector<bool> colorBlends;
	for (const auto &n: node["colorBlends"]) {
		colorBlends.push_back(n.as<bool>());
	}
	const auto subpass = node["subpass"].as<std::string>();
	return {
		id,
		vertexShader,
		fragmentShader,
		descSets,
		vertexInputAttributes,
		culling,
		colorBlends,
		subpassMap.at(subpass)
	};
}

Config parse(const YAML::Node yaml) {
	const auto title = yaml["title"].as<std::string>();
	const auto width = yaml["width"].as<int>();
	const auto height = yaml["height"].as<int>();

	std::unordered_map<std::string, uint32_t> attachmentMap;
	std::unordered_map<std::string, uint32_t> subpassMap;
	std::set<std::string> pipelineIDs;

	std::vector<Attachment> attachments;
	for (const auto &n : yaml["attachments"]) {
		attachments.push_back(parseAttachment(n, attachmentMap));
	}

	std::vector<Subpass> subpasses;
	for (const auto &n : yaml["subpasses"]) {
		subpasses.push_back(parseSubpass(n, attachmentMap, subpassMap));
	}

	std::vector<SubpassDependency> subpassDeps;
	if (yaml["subpass-deps"]) {
		for (const auto &n : yaml["subpass-deps"]) {
			const auto src = n["src"].as<std::string>();
			const auto dst = n["dst"].as<std::string>();
			subpassDeps.push_back({
				subpassMap.at(src),
				subpassMap.at(dst),
			});
		}
	}

	std::vector<Pipeline> pipelines;
	for (const auto &n: yaml["pipelines"]) {
		pipelines.push_back(parsePipeline(n, subpassMap));
		if (pipelineIDs.contains(pipelines.cend()->id)) {
			throw;
		} else {
			pipelineIDs.insert(pipelines.cend()->id);
		}
	}

	return {
		title,
		width,
		height,
		attachments,
		subpasses,
		subpassDeps,
		pipelines,
	};
}

std::optional<Config> parseConfig(const char *const yaml) {
	try {
		return parse(YAML::Load(yaml));
	} catch (...) {
		return std::nullopt;
	}
}

std::optional<Config> parseConfigFromFile(const char *const yamlFilePath) {
	try {
		return parse(YAML::LoadFile(yamlFilePath));
	} catch (...) {
		return std::nullopt;
	}
}
