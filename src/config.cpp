#include "config.hpp"

#include "graphics/platform.hpp"
#include "config/attachment.hpp"
#include "config/dependency.hpp"
#include "config/utils.hpp"

#include <format>

namespace config {

Config parse(const YAML::Node yaml) {
	const auto title = s(yaml, "title");
	const auto width = u(yaml, "width");
	const auto height = u(yaml, "height");

	std::unordered_map<std::string, uint32_t> attachmentMap;
	std::unordered_map<std::string, uint32_t> subpassMap;

	const auto attachments = parseAttachments(yaml, attachmentMap);
	const auto clearValues = parseClearValues(yaml);
	const auto subpasses = parseSubpasses(yaml, attachmentMap, subpassMap);
	const auto dependencies = parseDependencies(yaml, subpassMap);
	const auto pipelines = parsePipelines(yaml, subpassMap);

	return {
		title,
		width,
		height,
		attachments,
		clearValues,
		subpasses,
		dependencies,
		pipelines,
	};
}

Config parse(const char *const yaml) {
	return parse(YAML::Load(yaml));
}

Config parseFromFile(const char *const yamlFilePath) {
	try {
		return parse(YAML::LoadFile(yamlFilePath));
	} catch (const YAML::BadFile &) {
		throw std::format("config error: '{}' not found.", yamlFilePath);
	}
}

} // namespace config
