#pragma once

#include <unordered_map>
#include <yaml-cpp/yaml.h>

namespace config {

struct MeshConfig {
	const std::string vertices;
	const std::string indices;

	MeshConfig(const YAML::Node &node);
};

std::unordered_map<std::string, MeshConfig> parseMeshConfigs(const YAML::Node &node);

} // namespace config
