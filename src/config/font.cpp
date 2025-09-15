#include "font.hpp"

#include "utils.hpp"

namespace config {

FontConfig::FontConfig(const YAML::Node &node):
	file(s(node, "file")),
	charSize(u(node, "char-size")),
	charAtlusCol(u(node, "char-atlus-col")),
	charAtlusRow(u(node, "char-atlus-row"))
{
	checkUnexpectedKeys(node, {"id", "file", "char-size", "char-atlus-col", "char-atlus-row"});

	if (charSize == 0) {
		throw "config error: 'char-size' must be greater than 0.";
	}
	if (charAtlusCol == 0) {
		throw "config error: 'char-atlus-col' must be greater than 0.";
	}
	if (charAtlusRow == 0) {
		throw "config error: 'char-atlus-row' must be greater than 0.";
	}
}

std::unordered_map<std::string, FontConfig> parseFontConfigs(const YAML::Node &node) {
	std::unordered_map<std::string, FontConfig> fonts;
	for (const auto &n: node["fonts"]) {
		const auto id = s(n, "id");
		if (fonts.contains(id)) {
			throw std::format("config error: font '{}' duplicated.", id);
		}
		fonts.emplace(id, n);
	}
	return fonts;
}

} // namespace config
