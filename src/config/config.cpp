#include "config.hpp"

#include "../asset/asset.hpp"

#include <format>
#include <set>

namespace config {

std::optional<Config> g_config;

void checkUnexpectedKeys(const YAML::Node &n, const std::set<std::string> &ks) {
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

bool b(const YAML::Node &n, const std::string &k, std::optional<bool> d = std::nullopt) {
	return get<bool>(n, k, "bool", d);
}

float f(const YAML::Node &n, const std::string &k) {
	return get<float>(n, k, "float");
}

uint32_t u(const YAML::Node &n, const std::string &k, std::optional<uint32_t> d = std::nullopt) {
	return get<uint32_t>(n, k, "unsigned int", d);
}

std::string s(const YAML::Node &n, const std::string &k, std::optional<std::string> d = std::nullopt) {
	return get<std::string>(n, k, "string", d);
}

std::vector<bool> bs(const YAML::Node &n, const std::string &k) {
	return get<std::vector<bool>>(n, k, "bool[]");
}

std::vector<uint32_t> us(const YAML::Node &n, const std::string &k) {
	return get<std::vector<uint32_t>>(n, k, "unsigned int[]");
}

std::vector<std::string> ss(
	const YAML::Node &n,
	const std::string &k,
	std::optional<std::vector<std::string>> d = std::nullopt
) {
	return get<std::vector<std::string>>(n, k, "string[]", d);
}

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

DescriptorType parseDescriptorType(const std::string& s) {
	return s == "texture"
		? DescriptorType::Texture
		: s == "sampler"
		? DescriptorType::Sampler
		: s == "uniform-buffer"
		? DescriptorType::UniformBuffer
		: s == "storage-buffer"
		? DescriptorType::StorageBuffer
		: s == "input-attachment"
		? DescriptorType::InputAttachment
		: throw std::format("config error: type '{}' is invalid.", s);
}

ShaderStages parseShaderStages(const std::string& s) {
	return s == "vertex"
		? ShaderStages::Vertex
		: s == "fragment"
		? ShaderStages::Fragment
		: s == "vertex-and-fragment"
		? ShaderStages::VertexAndFragment
		: throw std::format("config error: stages '{}' is invalid.", s);
}

template<typename T>
std::vector<T> parseConfigs(const YAML::Node &node, const std::string &k) {
	std::vector<T> results;
	for (const auto &n: node[k]) {
		results.emplace_back(n);
	}
	return results;
}

AttachmentConfig::AttachmentConfig(const YAML::Node &node):
	id(s(node, "id")),
	format(parseFormat(s(node, "format"))),
	discard(b(node, "discard", false)),
	colorClearValue(node["clear-value"] && node["clear-value"].IsSequence()
		? std::make_optional(get<std::array<float, 4>>(node, "clear-value", "float[4]"))
		: std::nullopt),
	depthClearValue(node["clear-value"] && !node["clear-value"].IsSequence()
		? std::make_optional(f(node, "clear-value"))
		: std::nullopt)
{
	checkUnexpectedKeys(node, {"id", "format", "discard", "clear-value"});

	if (!node["clear-value"]) {
		throw "config error: 'clear-value' not found.";
	}
}

SubpassDepthConfig::SubpassDepthConfig(const YAML::Node &node): id(s(node, "id")), readOnly(b(node, "read-only")) {
	checkUnexpectedKeys(node, {"id", "read-only"});
}

SubpassConfig::SubpassConfig(const YAML::Node &node):
	id(s(node, "id")),
	inputs(ss(node, "inputs", std::make_optional<std::vector<std::string>>({}))),
	outputs(ss(node, "outputs")),
	depth(node["depth"] ? std::make_optional<SubpassDepthConfig>(node["depth"]) : std::nullopt),
	depends(ss(node, "depends", std::make_optional<std::vector<std::string>>({})))
{
	checkUnexpectedKeys(node, {"id", "inputs", "outputs", "depth", "depends"});
}

DescriptorBindingConfig::DescriptorBindingConfig(const YAML::Node &node):
	type(parseDescriptorType(s(node, "type"))),
	count(u(node, "count", 1)),
	stage(parseShaderStages(s(node, "stage"))),
	attachment(node["attachment"] ? std::make_optional(s(node, "attachment")) : std::nullopt)
{
	checkUnexpectedKeys(node, {"type", "count", "stage", "attachment"});

	if (type == DescriptorType::InputAttachment && !attachment.has_value()) {
		throw "config error: 'attachment' must be set if descriptor type is 'input-attachment'.";
	}
	if (attachment.has_value() && type != DescriptorType::InputAttachment && type != DescriptorType::Texture) {
		throw "config error: 'type' must be 'input-attachment' or 'texture' if 'attachment' is defined.";
	}
}

DescriptorSetConfig::DescriptorSetConfig(const YAML::Node &node):
	count(u(node, "count")),
	bindings(parseConfigs<DescriptorBindingConfig>(node, "bindings"))
{
	checkUnexpectedKeys(node, {"count", "bindings"});
}

PipelineConfig::PipelineConfig(const YAML::Node &node, const std::string &id):
	id(id),
	vertexShader(s(node, "vertex-shader")),
	fragmentShader(s(node, "fragment-shader")),
	descSets(parseConfigs<DescriptorSetConfig>(node, "desc-sets")),
	vertexInputAttributes(us(node, "vertex-input-attributes")),
	culling(b(node, "culling", false)),
	depthTest(b(node, "depth-test", false)),
	colorBlends(bs(node, "color-blends")),
	subpass(s(node, "subpass")),
	textRendering(false),
	charCount(0)
{
	checkUnexpectedKeys(
		node,
		{
			"id", "vertex-shader", "fragment-shader", "desc-sets", "vertex-input-attributes",
			"culling", "depth-test", "color-blends", "subpass"
		}
	);

	for (const auto &n: vertexInputAttributes) {
		validateValue(n, {1, 2, 3, 4}, "vertex input attribute");
	}
}

std::vector<DescriptorSetConfig> createTextRenderingPipelineDescSets(uint32_t texCount) {
	std::vector<DescriptorSetConfig> descSets;

	std::vector<DescriptorBindingConfig> bindings1;
	bindings1.emplace_back(DescriptorType::StorageBuffer, 1, ShaderStages::Vertex);
	descSets.emplace_back(1, std::move(bindings1));

	std::vector<DescriptorBindingConfig> bindings2;
	bindings2.emplace_back(DescriptorType::Texture, texCount, ShaderStages::Fragment);
	bindings2.emplace_back(DescriptorType::Sampler, 1, ShaderStages::Fragment);
	descSets.emplace_back(1, std::move(bindings2));

	return descSets;
}

PipelineConfig::PipelineConfig(const YAML::Node &node, uint32_t texCount):
	id(s(node, "id")),
	vertexShader(""),
	fragmentShader(""),
	descSets(createTextRenderingPipelineDescSets(texCount)),
	vertexInputAttributes(),
	culling(false),
	depthTest(false),
	colorBlends{true},
	subpass(s(node, "subpass")),
	textRendering(true),
	charCount(u(node, "char-count"))
{
	checkUnexpectedKeys(node, {"id", "text-rendering", "subpass", "char-count"});
}

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

template<typename T>
std::unordered_map<std::string, uint32_t> collectMap(const std::vector<T> &v) {
	std::unordered_map<std::string, uint32_t> map;
	for (const auto &n: v) {
		map.emplace(n.id, static_cast<uint32_t>(map.size()));
	}
	return map;
}

std::vector<PipelineConfig> parsePipelineConfigs(const YAML::Node &node, uint32_t texCount) {
	std::vector<PipelineConfig> results;
	for (const auto &n: node["pipelines"]) {
		if (b(n, "text-rendering", false)) {
			results.emplace_back(n, texCount);
		} else {
			results.emplace_back(n, s(n, "id"));
		}
	}
	return results;
}

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

Config::Config(YAML::Node node):
	title(s(node, "title")),
	width(u(node, "width")),
	height(u(node, "height")),
	fullscreen(b(node, "fullscreen", false)),
	altReturnToggleFullscreen(b(node, "alt-return-toggle-fullscreen", true)),
	audioChannelCount(u(node, "audio-channel-count", 16)),
	fonts(parseConfigs<FontConfig>(node, "fonts")),
	attachments(parseConfigs<AttachmentConfig>(node, "attachments")),
	subpasses(parseConfigs<SubpassConfig>(node, "subpasses")),
	pipelines(parsePipelineConfigs(node, static_cast<uint32_t>(fonts.size()))),
	attachmentMap(collectMap(attachments)),
	subpassMap(collectMap(subpasses)),
	fontMap(collectMap(fonts)),
	assetMap(collectAssetMap(node))
{
	checkUnexpectedKeys(
		node,
		{
			"title",
			"width",
			"height",
			"fullscreen",
			"alt-return-toggle-fullscreen",
			"audio-channel-count",
			"attachments",
			"subpasses",
			"pipelines",
			"fonts",
			"assets",
		}
	);

	for (const auto &n: subpasses) {
		for (const auto &m: n.inputs) {
			if (!attachmentMap.contains(m)) {
				throw std::format("config error: attachment '{}' not defined.", m);
			}
		}
		for (const auto &m: n.outputs) {
			if (!attachmentMap.contains(m)) {
				throw std::format("config error: attachment '{}' not defined.", m);
			}
		}
		if (n.depth.has_value()) {
			const auto &id = n.depth->id;
			if (!attachmentMap.contains(id)) {
				throw std::format("config error: attachment '{}' not defined.", id);
			}
		}
		for (const auto &m: n.depends) {
			if (!subpassMap.contains(m)) {
				throw std::format("config error: subpass '{}' not defined.", m);
			}
		}
	}
	std::set<std::string> textRenderingPipelineSubpasses;
	for (const auto &n: pipelines) {
		for (const auto &m: n.descSets) {
			for (const auto &x: m.bindings) {
				if (x.attachment.has_value()) {
					const auto &id = x.attachment.value();
					if (!attachmentMap.contains(id)) {
						throw std::format("config error: attachment '{}' not defined.", id);
					}
				}
			}
		}
		if (!subpassMap.contains(n.subpass)) {
			throw std::format("config error: subpass '{}' not defined.", n.subpass);
		}
		if (n.textRendering) {
			if (textRenderingPipelineSubpasses.contains(n.subpass)) {
				throw std::format("config error: Subpasses corresponding to the text rendering pipeline must be injective: {}", n.subpass);
			}
			textRenderingPipelineSubpasses.emplace(n.subpass);
		}
	}
}

void initialize() {
	const auto data = asset::getAsset(0);
	const auto yaml = std::string(data.cbegin(), data.cend());
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
