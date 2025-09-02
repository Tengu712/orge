#include "renderpass.hpp"

#include "utils.hpp"

namespace config {

SubpassDepthConfig::SubpassDepthConfig(const YAML::Node &node): id(s(node, "id")), readOnly(b(node, "read-only")) {
	checkUnexpectedKeys(node, {"id", "read-only"});
}

SubpassConfig::SubpassConfig(const YAML::Node &node):
	id(s(node, "id")),
	inputs(sus(node, "inputs", std::make_optional<std::unordered_set<std::string>>({}))),
	outputs(sus(node, "outputs")),
	depth(node["depth"] ? std::make_optional<SubpassDepthConfig>(node["depth"]) : std::nullopt),
	depends(sus(node, "depends", std::make_optional<std::unordered_set<std::string>>({}))),
	pipelines(sus(node, "pipelines"))
{
	checkUnexpectedKeys(node, {"id", "inputs", "outputs", "depth", "depends", "pipelines"});
}

std::vector<std::string> collectAttachments(const std::vector<SubpassConfig> &subpasses) {
	std::vector<std::string> attachments;
	for (const auto &n: subpasses) {
		for (const auto &m: n.inputs) {
			attachments.push_back(m);
		}
		for (const auto &m: n.outputs) {
			attachments.push_back(m);
		}
		if (n.depth) {
			attachments.push_back(n.depth.value().id);
		}
	}
	return attachments;
}

RenderPassConfig::RenderPassConfig(const YAML::Node &node):
	subpasses(parseConfigs<SubpassConfig>(node, "subpasses")),
	attachments(collectAttachments(subpasses)),
	attachmentMap(collectMap(attachments)),
	subpassMap(collectMap(subpasses))
{
	checkUnexpectedKeys(node, {"id", "subpasses"});

	// TODO: 順番もバリデーションしたい。
	for (const auto &n: subpasses) {
		for (const auto &m: n.depends) {
			if (!subpassMap.contains(m)) {
				throw std::format("config error: subpass '{}' not defined.", m);
			}
		}
	}
}

std::unordered_map<std::string, RenderPassConfig> parseRenderPassConfigs(const YAML::Node &node) {
	std::unordered_map<std::string, RenderPassConfig> renderPasses;
	for (const auto &n: node["render-passes"]) {
		const auto id = s(n, "id");
		if (renderPasses.contains(id)) {
			throw std::format("config error: render pass '{}' duplicated.", id);
		}
		renderPasses.emplace(id, RenderPassConfig(n));
	}
	return renderPasses;
}

} // namespace config
