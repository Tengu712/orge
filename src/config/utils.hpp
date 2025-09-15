#pragma once

#include <format>
#include <unordered_set>
#include <optional>
#include <yaml-cpp/yaml.h>

namespace config {

inline void checkUnexpectedKeys(const YAML::Node &n, const std::set<std::string> &ks) {
	for (const auto &p: n) {
		const auto k = p.first.as<std::string>();
		if (!ks.contains(k)) {
			throw std::format("config error: unexpected key '{}' found.", k);
		}
	}
}

template<typename T>
void validateValue(const T &v, const std::set<T> &cs, const std::string &s) {
	if (!cs.contains(v)) {
		throw std::format("config error: {} '{}' is invalid.", s, v);
	}
}

template<typename T>
T get(const YAML::Node &n, const std::string &k, const std::string &t, std::optional<T> d = std::nullopt) {
	if (!n[k]) {
		if (d) {
			return d.value();
		} else {
			throw std::format("config error: '{}' not found.", k);
		}
	}
	try {
		return n[k].as<T>();
	} catch (...) {
		throw std::format("config error: '{}' is not a {}.", k, t);
	}
}

inline bool b(const YAML::Node &n, const std::string &k, std::optional<bool> d = std::nullopt) {
	return get<bool>(n, k, "bool", d);
}

inline float f(const YAML::Node &n, const std::string &k) {
	return get<float>(n, k, "float");
}

inline uint32_t u(const YAML::Node &n, const std::string &k, std::optional<uint32_t> d = std::nullopt) {
	return get<uint32_t>(n, k, "unsigned int", d);
}

inline std::string s(const YAML::Node &n, const std::string &k, std::optional<std::string> d = std::nullopt) {
	return get<std::string>(n, k, "string", d);
}

inline std::vector<bool> bs(const YAML::Node &n, const std::string &k) {
	return get<std::vector<bool>>(n, k, "bool[]");
}

inline std::vector<uint32_t> us(
	const YAML::Node &n,
	const std::string &k,
	std::optional<std::vector<std::uint32_t>> d = std::nullopt
) {
	return get<std::vector<uint32_t>>(n, k, "unsigned int[]", d);
}

inline std::vector<std::string> ss(
	const YAML::Node &n,
	const std::string &k,
	std::optional<std::vector<std::string>> d = std::nullopt
) {
	return get<std::vector<std::string>>(n, k, "string[]", d);
}

inline std::unordered_set<std::string> sus(
	const YAML::Node &n,
	const std::string &k,
	std::optional<std::unordered_set<std::string>> d = std::nullopt
) {
	if (!n[k]) {
		if (d) {
			return d.value();
		} else {
			throw std::format("config error: '{}' not found.", k);
		}
	}
	std::unordered_set<std::string> results;
	for (const auto &m: n[k]) {
		try {
			results.emplace(m.as<std::string>());
		} catch (...) {
			throw std::format("config error: '{}' is not a string.", k);
		}
	}
	return results;
}

template<typename T>
std::vector<T> parseConfigs(const YAML::Node &node, const std::string &k) {
	std::vector<T> results;
	for (const auto &n: node[k]) {
		results.emplace_back(n);
	}
	return results;
}

template<typename T>
std::unordered_map<std::string, uint32_t> collectMap(const std::vector<T> &v) {
	std::unordered_map<std::string, uint32_t> map;
	for (const auto &n: v) {
		// TODO: サブジェクトを追加したい。
		if (map.contains(n.id)) {
			throw std::format("config error: '{}' duplicated.", n.id);
		}
		map.emplace(n.id, static_cast<uint32_t>(map.size()));
	}
	return map;
}

template<>
inline std::unordered_map<std::string, uint32_t> collectMap<std::string>(const std::vector<std::string> &v) {
	std::unordered_map<std::string, uint32_t> map;
	for (const auto &n: v) {
		// TODO: サブジェクトを追加したい。
		if (map.contains(n)) {
			throw std::format("config error: '{}' duplicated.", n);
		}
		map.emplace(n, static_cast<uint32_t>(map.size()));
	}
	return map;
}

} // namespace config
