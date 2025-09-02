#include "font.hpp"

#include "utils.hpp"

namespace config {

FontConfig::FontConfig(const YAML::Node &node):
	id(s(node, "id")),
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

} // namespace config
