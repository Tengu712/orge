#include "config.hpp"

#include "../asset/asset.hpp"
#include "utils.hpp"

namespace config {

std::optional<Config> g_config;

std::unordered_map<std::string, uint32_t> collectAssetMap(const YAML::Node &node) {
	if (!node["assets"]) {
		return {};
	}
	if (!node["assets"].IsSequence()) {
		throw "config error: assets must be a string sequence.";
	}

	const auto assets = ss(node, "assets");

	std::unordered_map<std::string, uint32_t> assetMap;
	for (const auto &n: assets) {
		if (assetMap.contains(n)) {
			throw std::format("config error: asset '{}' is duplicated.", n);
		}
		const auto size = assetMap.size();
		assetMap.emplace(n, static_cast<uint32_t>(size));
	}

	return assetMap;
}

Config::Config(const YAML::Node &node):
	title(s(node, "title")),
	width(u(node, "width")),
	height(u(node, "height")),
	fullscreen(b(node, "fullscreen", false)),
	disableVsync(b(node, "disable-vsync", false)),
	altReturnToggleFullscreen(b(node, "alt-return-toggle-fullscreen", true)),
	audioChannelCount(u(node, "audio-channel-count", 16)),
	meshes(parseMeshConfigs(node)),
	fonts(parseConfigs<FontConfig>(node, "fonts")),
	attachments(parseAttachmentConfigs(node)),
	pipelines(parsePipelineConfigs(node, static_cast<uint32_t>(fonts.size()))),
	renderPasses(parseRenderPassConfigs(node)),
	assetMap(collectAssetMap(node)),
	fontMap(collectMap(fonts))
{
	checkUnexpectedKeys(
		node,
		{
			"title",
			"width",
			"height",
			"fullscreen",
			"disable-vsync",
			"alt-return-toggle-fullscreen",
			"audio-channel-count",
			"assets",
			"meshes",
			"fonts",
			"attachments",
			"pipelines",
			"render-passes",
		}
	);

	for (const auto &[_, n]: pipelines) {
		// アタッチメントバインディングの存在確認
		for (const auto &m: n.descSets) {
			for (const auto &x: m.bindings) {
				if (x.attachment.has_value()) {
					const auto &id = x.attachment.value();
					if (!attachments.contains(id)) {
						throw std::format("config error: attachment '{}' not defined.", id);
					}
				}
			}
		}
	}

	std::set<std::string> renderPassIds;
	for (const auto &[_, r]: renderPasses) {
		for (const auto &n: r.subpasses) {
			// 各アタッチメントの確認
			for (const auto &m: n.inputs) {
				if (!attachments.contains(m)) {
					throw std::format("config error: attachment '{}' not defined.", m);
				}
				switch (attachments.at(m).format) {
				case Format::DepthBuffer:
				case Format::ShareColorAttachment:
				case Format::SignedShareColorAttachment:
					break;
				default:
					throw "config error: the attachment format of a subpass input must be 'depth-buffer', 'share-color-attachment' or 'signed-share-color-attachment'.";
				};
			}
			for (const auto &m: n.outputs) {
				if (!attachments.contains(m)) {
					throw std::format("config error: attachment '{}' not defined.", m);
				}
				// TODO: ヴァリデーション。
			}
			if (n.depth.has_value()) {
				const auto &id = n.depth->id;
				if (!attachments.contains(id)) {
					throw std::format("config error: attachment '{}' not defined.", id);
				}
				// TODO: ヴァリデーション。
			}
			// パイプラインの存在確認
			for (const auto &m: n.pipelines) {
				if (!pipelines.contains(m)) {
					throw std::format("config error: pipeline '{}' not defined.", m);
				}
			}
		}
	}
}

void initialize() {
	const auto data = asset::getConfigData();
	const auto yaml = std::string(reinterpret_cast<const char *>(data.data()), data.size());
	g_config.emplace(YAML::Load(yaml));
}

const Config &config() {
	if (g_config.has_value()) {
		return g_config.value();
	} else {
		throw "config not initialized.";
	}
}

} // namespace config
