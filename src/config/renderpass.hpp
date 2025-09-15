#pragma once

#include <optional>
#include <unordered_set>
#include <yaml-cpp/yaml.h>

namespace config {

struct SubpassDepthConfig {
	const std::string id;
	const bool readOnly;

	SubpassDepthConfig(const YAML::Node &node);
};

struct SubpassConfig {
	const std::string id;
	const std::unordered_set<std::string> inputs;
	const std::unordered_set<std::string> outputs;
	const std::optional<SubpassDepthConfig> depth;
	const std::unordered_set<std::string> depends;
	const std::unordered_set<std::string> pipelines;

	SubpassConfig(const YAML::Node &node);
};

struct RenderPassConfig {
	const std::vector<SubpassConfig> subpasses;

	const std::vector<std::string> attachments;
	const std::unordered_map<std::string, uint32_t> attachmentMap;
	const std::unordered_map<std::string, uint32_t> subpassMap;

	RenderPassConfig(const YAML::Node &node);
};

std::unordered_map<std::string, RenderPassConfig> parseRenderPassConfigs(const YAML::Node &node);

} // namespace config
