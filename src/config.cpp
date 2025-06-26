#include "config.hpp"

#include <yaml-cpp/yaml.h>

std::optional<Config> parse(const YAML::Node yaml) {
	const auto title = yaml["title"].as<std::string>();
	const auto width = yaml["width"].as<int>();
	const auto height = yaml["height"].as<int>();
	return std::optional<Config>({
		title,
		width,
		height,
	});
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
