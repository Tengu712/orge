#pragma once

#include <yaml-cpp/yaml.h>

namespace config {

struct FontConfig {
	const std::string file;
	const uint32_t charSize;
	const uint32_t charAtlusCol;
	const uint32_t charAtlusRow;

	FontConfig(const YAML::Node &node);
};

std::unordered_map<std::string, FontConfig> parseFontConfigs(const YAML::Node &node);

} // namespace config
