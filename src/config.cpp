#include "config.hpp"

#include "graphics/platform.hpp"

#include <unordered_map>
#include <yaml-cpp/yaml.h>

Attachment parseAttachment(const YAML::Node &node, std::unordered_map<std::string, uint32_t> &attachmentMap) {
	const auto id = node["id"].as<std::string>();
	if (!attachmentMap.emplace(id, attachmentMap.size()).second) {
		throw;
	}
	const auto format = node["format"].as<std::string>();
	const auto discard = node["discard"].as<bool>(false);
	const auto finalLayout = node["final-layout"].as<std::string>();
	return {
		// TODO: parse format
		graphics::platform::getRenderTargetPixelFormat(),
		discard,
		finalLayout == "general" ? vk::ImageLayout::eGeneral
		: finalLayout == "color-attachment" ? vk::ImageLayout::eColorAttachmentOptimal
		: finalLayout == "depth-stencil-attachment" ? vk::ImageLayout::eDepthStencilAttachmentOptimal
		: finalLayout == "transfer-src" ? vk::ImageLayout::eTransferSrcOptimal
		: finalLayout == "present-src" ? vk::ImageLayout::ePresentSrcKHR
		: throw,
	};
}

Subpass parseSubpass(const YAML::Node &node, const std::unordered_map<std::string, uint32_t> &attachmentMap, std::unordered_map<std::string, uint32_t> &subpassMap) {
	const auto id = node["id"].as<std::string>();
	if (!subpassMap.emplace(id, subpassMap.size()).second) {
		throw;
	}

	std::vector<SubpassAttachment> inputs;
	if (node["inputs"]) {
		for (const auto &n : node["inputs"]) {
			const auto id = n["id"].as<std::string>();
			const auto layout = n["layout"].as<std::string>();
			inputs.push_back({
				attachmentMap.at(id),
				layout == "general" ? vk::ImageLayout::eGeneral
				: layout == "depth-stencil-read-only" ? vk::ImageLayout::eDepthStencilReadOnlyOptimal
				: layout == "shader-read-only" ? vk::ImageLayout::eShaderReadOnlyOptimal
				: throw,
			});
		}
	}

	std::vector<SubpassAttachment> outputs;
	for (const auto &n : node["outputs"]) {
		const auto id = n["id"].as<std::string>();
		const auto layout = n["layout"].as<std::string>();
		inputs.push_back({
			attachmentMap.at(id),
			layout == "general" ? vk::ImageLayout::eGeneral
			: layout == "color-attachment" ? vk::ImageLayout::eColorAttachmentOptimal
			: throw,
		});
	}

	const std::optional<SubpassAttachment> depth = !node["depth"]
		? std::nullopt
		: [&]() -> std::optional<SubpassAttachment> {
			const auto &n = node["depth"];
			const auto id = n["id"].as<std::string>();
			const auto layout = n["layout"].as<std::string>();
			return SubpassAttachment {
				attachmentMap.at(id),
				layout == "general" ? vk::ImageLayout::eGeneral
				: layout == "depth-stencil-attachment" ? vk::ImageLayout::eDepthStencilAttachmentOptimal
				: layout == "depth-stencil-read-only" ? vk::ImageLayout::eDepthStencilReadOnlyOptimal
				: throw,
			};
		}();

	return {
		inputs,
		outputs,
		depth,
	};
}

Config parse(const YAML::Node yaml) {
	const auto title = yaml["title"].as<std::string>();
	const auto width = yaml["width"].as<int>();
	const auto height = yaml["height"].as<int>();

	std::unordered_map<std::string, uint32_t> attachmentMap;
	std::unordered_map<std::string, uint32_t> subpassMap;

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

	return {
		title,
		width,
		height,
		attachments,
		subpasses,
		subpassDeps,
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
