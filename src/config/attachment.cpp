#include "attachment.hpp"

#include "utils.hpp"

#include <format>

namespace config {

Format parseFormat(const std::string& s) {
	return s == "render-target"
		? Format::RenderTarget
		: s == "depth-buffer"
		? Format::DepthBuffer
		: s == "share-color-attachment"
		? Format::ShareColorAttachment
		: s == "signed-share-color-attachment"
		? Format::SignedShareColorAttachment
		: throw std::format("config error: format '{}' is invalid.", s);
}

ClearValueConfig parseClearValueConfig(const YAML::Node &node) {
	const bool isColor = node["clear-value"] && node["clear-value"].IsSequence();
	return isColor
		? ClearValueConfig(get<std::array<float, 4>>(node, "clear-value", "float[4]"))
		: ClearValueConfig(f(node, "clear-value"));
}

AttachmentConfig::AttachmentConfig(const YAML::Node &node):
	format(parseFormat(s(node, "format"))),
	discard(b(node, "discard", false)),
	clearValue(parseClearValueConfig(node))
{
	checkUnexpectedKeys(node, {"id", "format", "discard", "clear-value"});
}

std::unordered_map<std::string, AttachmentConfig> parseAttachmentConfigs(const YAML::Node &node) {
	std::unordered_map<std::string, AttachmentConfig> attachments;
	for (const auto &n: node["attachments"]) {
		const auto id = s(n, "id");
		if (attachments.contains(id)) {
			throw std::format("config error: attachments '{}' duplicated.", id);
		}
		attachments.emplace(id, AttachmentConfig(n));
	}
	return attachments;
}

} // namespace config
