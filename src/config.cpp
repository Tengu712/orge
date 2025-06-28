#include "config.hpp"

#include "graphics/platform.hpp"

#include <unordered_map>
#include <yaml-cpp/yaml.h>

Attachment parseAttachment(const YAML::Node &node, std::unordered_map<std::string, uint32_t> &attachmentMap) {
	const auto id = node["id"].as<std::string>();
	if (!attachmentMap.contains(id)) {
		attachmentMap[id] = attachmentMap.size();
	} else {
		throw;
	}
	const auto format_ = node["format"].as<std::string>();
	// TODO: parse format
	const auto format = graphics::platform::getRenderTargetPixelFormat();
	const auto discard = node["discard"].as<bool>(false);
	const auto finalLayout_ = node["final-layout"].as<std::string>();
	const auto finalLayout =
		finalLayout_ == "general" ? vk::ImageLayout::eGeneral
		: finalLayout_ == "color-attachment" ? vk::ImageLayout::eColorAttachmentOptimal
		: finalLayout_ == "depth-stencil-attachment" ? vk::ImageLayout::eDepthStencilAttachmentOptimal
		: finalLayout_ == "transfer-src" ? vk::ImageLayout::eTransferSrcOptimal
		: finalLayout_ == "present-src" ? vk::ImageLayout::ePresentSrcKHR
		: throw;
	return {
		format,
		discard,
		finalLayout,
	};
}

Config parse(const YAML::Node yaml) {
	const auto title = yaml["title"].as<std::string>();
	const auto width = yaml["width"].as<int>();
	const auto height = yaml["height"].as<int>();

	std::unordered_map<std::string, uint32_t> attachmentMap;
	std::unordered_map<std::string, uint32_t> subpassMap;
	std::vector<Attachment> attachments;
	const auto attachments_ = yaml["attachments"];
	std::vector<Subpass> subpasses;
	std::vector<SubpassDependency> subpassDeps;

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
