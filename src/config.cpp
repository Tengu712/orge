#include "config.hpp"

#include "graphics/platform.hpp"

#include <format>
#include <set>
#include <unordered_map>
#include <yaml-cpp/yaml.h>

Attachment parseAttachment(const YAML::Node &node, std::unordered_map<std::string, uint32_t> &attachmentMap) {
	const auto id = node["id"].as<std::string>();
	if (!attachmentMap.emplace(id, static_cast<uint32_t>(attachmentMap.size())).second) {
		throw std::format("config error: attachment id '{}' is duplicated.", id);
	}
	const auto format = node["format"].as<std::string>();
	const auto discard = node["discard"].as<bool>(false);
	const auto finalLayout = node["final-layout"].as<std::string>();
	const auto clearValue = node["clear-value"];
	return {
		format == "render-target" ? graphics::platform::getRenderTargetPixelFormat()
		: throw std::format("config error: attachment format '{}' is invalid.", format),
		discard,
		finalLayout == "general" ? vk::ImageLayout::eGeneral
		: finalLayout == "color-attachment" ? vk::ImageLayout::eColorAttachmentOptimal
		: finalLayout == "depth-stencil-attachment" ? vk::ImageLayout::eDepthStencilAttachmentOptimal
		: finalLayout == "transfer-src" ? vk::ImageLayout::eTransferSrcOptimal
		: finalLayout == "present-src" ? vk::ImageLayout::ePresentSrcKHR
		: throw std::format("config error: attachment final layout '{}' is invalid.", finalLayout),
		clearValue.IsSequence() && clearValue.size() == 4
		? static_cast<vk::ClearValue>(vk::ClearColorValue(clearValue[0].as<float>(), clearValue[1].as<float>(), clearValue[2].as<float>(), clearValue[3].as<float>()))
		: clearValue.IsScalar()
		? static_cast<vk::ClearValue>(vk::ClearDepthStencilValue(clearValue.as<float>(), 0))
		: throw std::format("config error: attachment clear value format is invalid."),
	};
}

Subpass parseSubpass(const YAML::Node &node, const std::unordered_map<std::string, uint32_t> &attachmentMap, std::unordered_map<std::string, uint32_t> &subpassMap) {
	const auto id = node["id"].as<std::string>();
	if (!subpassMap.emplace(id, static_cast<uint32_t>(subpassMap.size())).second) {
		throw std::format("config error: subpass id '{}' is duplicated.", id);
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
				: throw std::format("config error: subpass input layout '{}' is invalid.", layout)
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
			: throw std::format("config error: subpass output layout '{}' is invalid.", layout)
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
			: throw std::format("config error: subpass depth layout '{}' is invalid.", layout)
		);
	}

	return {
		inputs,
		outputs,
		depth,
	};
}

PipelineConfig parsePipeline(const YAML::Node &node, const std::unordered_map<std::string, uint32_t> &subpassMap) {
	const auto id = node["id"].as<std::string>();

	const auto vertexShader = node["vertexShader"].as<std::string>();
	const auto fragmentShader = node["fragmentShader"].as<std::string>();

	std::vector<DescriptorSetConfig> descSets;
	for (const auto &m: node["desc-sets"]) {
		const auto count = m["count"].as<uint32_t>();

		std::vector<vk::DescriptorSetLayoutBinding> bindings;
		for (const auto &n: m["bindings"]) {
			const auto type = n["type"].as<std::string>();
			const auto count = n["count"].as<uint32_t>(1);

			vk::ShaderStageFlags stages{};
			for (const auto &s: n["stages"]) {
				const auto t = s.as<std::string>();
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
				: throw std::format("config error: pipeline descriptor type '{}' is invalid.", type),
				count,
				stages,
				nullptr
			);
		}

		descSets.push_back(DescriptorSetConfig {count, std::move(bindings)});
	}

	std::vector<uint32_t> vertexInputAttributes;
	for (const auto &n: node["vertexInputAttributes"]) {
		const auto vertexInputAttribute = n.as<uint32_t>();
		if (vertexInputAttribute < 1 || vertexInputAttribute > 4) {
			throw std::format("config error: pipeline vertex input attribute '{}' is invalid (must be 1-4).", vertexInputAttribute);
		}
		vertexInputAttributes.push_back(vertexInputAttribute);
	}
	if (vertexInputAttributes.empty()) {
		throw std::format("config error: pipeline vertex input attributes cannot be empty.");
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

	std::vector<PipelineConfig> pipelines;
	for (const auto &n: yaml["pipelines"]) {
		pipelines.push_back(parsePipeline(n, subpassMap));
		if (pipelineIDs.contains(pipelines.cend()->id)) {
			throw std::format("config error: pipeline id '{}' is duplicated.", pipelines.cend()->id);
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

Config parseConfig(const char *const yaml) {
	try {
		return parse(YAML::Load(yaml));
	} catch (const std::string &e) {
		throw e;
	} catch (const YAML::BadConversion) {
		// TODO: どのキーが定義されていないのか含める。
		throw "config error: some key is undefined.";
	}
}

Config parseConfigFromFile(const char *const yamlFilePath) {
	try {
		return parse(YAML::LoadFile(yamlFilePath));
	} catch (const std::string &e) {
		throw e;
	} catch (const YAML::BadConversion &) {
		// TODO: どのキーが定義されていないのか含める。
		throw "config error: some key is undefined.";
	} catch (const YAML::BadFile &) {
		throw std::format("config error: '{}' not found.", yamlFilePath);
	}
}
