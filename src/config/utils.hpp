#pragma once

#include <format>
#include <yaml-cpp/yaml.h>

namespace config {

inline YAML::Node a(const YAML::Node &node, const char *name) {
	if (!node[name].IsSequence() || node[name].size() == 0) {
		throw std::format("config error: '{}' must be a non-empty array.", name);
	} else {
		return node[name];
	}
}

inline bool b(const YAML::Node &node, const char *name) {
	if (!node[name]) {
		throw std::format("config error: '{}' not found.", name);
	}
	try {
		return node[name].as<bool>();
	} catch (...) {
		throw std::format("config error: '{}' is not a bool.", name);
	}
}

inline uint32_t u(const YAML::Node &node, const char *name) {
	if (!node[name]) {
		throw std::format("config error: '{}' not found.", name);
	}
	try {
		return node[name].as<uint32_t>();
	} catch (...) {
		throw std::format("config error: '{}' is not a unsigned int.", name);
	}
}

inline std::string s(const YAML::Node &node, const char *name) {
	if (!node[name]) {
		throw std::format("config error: '{}' not found.", name);
	}
	try {
		return node[name].as<std::string>();
	} catch (...) {
		throw std::format("config error: '{}' is not a string.", name);
	}
}

inline bool cb(const YAML::Node &node, const char *name) {
	try {
		return node.as<bool>();
	} catch (...) {
		throw std::format("config error: {} is not a bool.", name);
	}
}

inline float cf(const YAML::Node &node, const char *name) {
	try {
		return node.as<float>();
	} catch (...) {
		throw std::format("config error: {} is not a float.", name);
	}
}

inline uint32_t cu(const YAML::Node &node, const char *name) {
	try {
		return node.as<uint32_t>();
	} catch (...) {
		throw std::format("config error: {} is not a unsigned int.", name);
	}
}

inline std::string cs(const YAML::Node &node, const char *name) {
	try {
		return node.as<std::string>();
	} catch (...) {
		throw std::format("config error: {} is not a string.", name);
	}
}

inline uint32_t at(
	const std::unordered_map<std::string, uint32_t> &map,
	const YAML::Node &node,
	const char *name
) {
	const auto key = s(node, name);
	if (map.contains(key)) {
		return map.at(key);
	} else {
		throw std::format("config error: {} '{}' is not defined.", name, key);
	}
}

} // namespace config
