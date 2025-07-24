#include "config.hpp"

#include <format>
#include <set>

namespace config {

void validateKeys(const YAML::Node &n, const std::set<std::string> &rs, const std::set<std::string> &os) {
	for (const auto &p: n) {
		const auto k = p.first.as<std::string>();
		if (!rs.contains(k) && !os.contains(k)) {
			throw std::format("config error: unexpected key '{}' found.", k);
		}
	}
	for (const auto &k: rs) {
		if (!n[k]) {
			throw std::format("config error: '{}' not found.", k);
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

uint32_t u(const YAML::Node &n, const std::string &k, std::optional<bool> d = std::nullopt) {
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

std::vector<std::string> ss(const YAML::Node &n, const std::string &k) {
	return get<std::vector<std::string>>(n, k, "string[]");
}

Format parseFormat(const std::string& s) {
	return s == "render-target"
		? Format::RenderTarget
		: s == "depth-buffer"
		? Format::DepthBuffer
		: s == "share-color-attachment"
		? Format::ShareColorAttachment
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

AttachmentConfig::AttachmentConfig(const YAML::Node &node) {
	validateKeys(node, {"id", "format", "clear-value"}, {"discard"});

	id = s(node, "id");
	format = parseFormat(s(node, "format"));
	discard = b(node, "discard", false);
	if (node["clear-value"].IsSequence()) {
		colorClearValue = get<std::array<float, 4>>(node, "clear-value", "float[4]");
	} else {
		depthClearValue = f(node, "clear-value");
	}
}

SubpassDepthConfig::SubpassDepthConfig(const YAML::Node &node) {
	validateKeys(node, {"id", "read-only"}, {});

	id = s(node, "id");
	readOnly = b(node, "read-only");
}

SubpassConfig::SubpassConfig(const YAML::Node &node) {
	validateKeys(node, {"id", "outputs"}, {"inputs", "depth", "depends"});

	id = s(node, "id");

	if (node["inputs"]) {
		inputs = ss(node, "inputs");
	}

	outputs = ss(node, "outputs");

	if (node["depth"]) {
		depth = SubpassDepthConfig(node["depth"]);
	}

	if (node["depends"]) {
		depends = ss(node, "depends");
	}
}

DescriptorBindingConfig::DescriptorBindingConfig(const YAML::Node &node) {
	validateKeys(node, {"type", "stage"}, {"count", "attachment"});

	type = parseDescriptorType(s(node, "type"));
	count = u(node, "count", 1);
	stage = parseShaderStages(s(node, "stage"));

	if (node["attachment"]) {
		attachment = s(node, "attachment");
	}

	if (type == DescriptorType::InputAttachment && !attachment.has_value()) {
		throw "config error: 'attachment' must be set if descriptor type is 'input-attachment'.";
	}
	if (attachment.has_value() && type != DescriptorType::InputAttachment && type != DescriptorType::Texture) {
		throw "config error: 'type' must be 'input-attachment' or 'texture' if 'attachment' is defined.";
	}
}

DescriptorSetConfig::DescriptorSetConfig(const YAML::Node &node) {
	validateKeys(node, {"count", "bindings"}, {});

	count = u(node, "count");

	for (const auto &n: node["bindings"]) {
		bindings.emplace_back(n);
	}
}

PipelineConfig::PipelineConfig(const YAML::Node &node) {
	validateKeys(
		node,
		{"id", "vertex-shader", "fragment-shader", "vertex-input-attributes", "color-blends", "subpass"},
		{"desc-sets", "culling", "depth-test"}
	);

	id = s(node, "id");
	vertexShader = s(node, "vertex-shader");
	fragmentShader = s(node, "fragment-shader");
	vertexInputAttributes = us(node, "vertex-input-attributes");
	culling = b(node, "culling", false);
	depthTest = b(node, "depth-test", false);
	colorBlends = bs(node, "color-blends");
	subpass = s(node, "subpass");

	for (const auto &descSetNode: node["desc-sets"]) {
		descSets.emplace_back(descSetNode);
	}

	for (const auto &n: vertexInputAttributes) {
		validateValue(n, {1, 2, 3, 4}, "vertex input attribute");
	}
}

Config::Config(const YAML::Node &node) {
	validateKeys(
		node,
		{"title", "width", "height", "attachments", "subpasses"},
		{"fullscreen", "alt-tab-toggle-fullscreen", "pipelines"}
	);

	title = s(node, "title");
	width = u(node, "width");
	height = u(node, "height");
	fullscreen = b(node, "fullscreen", false);
	altTabToggleFullscreen = b(node, "alt-tab-toggle-fullscreen", true);

	for (const auto &n: node["attachments"]) {
		attachments.emplace_back(n);
	}

	for (const auto &n: node["subpasses"]) {
		subpasses.emplace_back(n);
	}

	for (const auto &n: node["pipelines"]) {
		pipelines.emplace_back(n);
	}

	for (const auto &n: attachments) {
		attachmentMap.emplace(n.id, static_cast<uint32_t>(attachmentMap.size()));
	}
	for (const auto &n: subpasses) {
		subpassMap.emplace(n.id, static_cast<uint32_t>(subpassMap.size()));
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
	}
}

Config parse(const char *yaml) {
	return Config(YAML::Load(yaml));
}

Config parseFromFile(const char *yamlFilePath) {
	try {
		return Config(YAML::LoadFile(yamlFilePath));
	} catch (const YAML::BadFile &) {
		throw std::format("config error: config file '{}' not found.", yamlFilePath);
	}
}

} // namespace config
