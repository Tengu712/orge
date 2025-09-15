#pragma once

#include <yaml-cpp/yaml.h>

#include <variant>

namespace config {

enum class Format: uint8_t {
	RenderTarget,
	DepthBuffer,
	ShareColorAttachment,
	SignedShareColorAttachment,
};

using ColorClearValue = std::array<float, 4>;
using DepthClearValue = float;
using ClearValueConfig = std::variant<ColorClearValue, DepthClearValue>;

struct AttachmentConfig {
	const Format format;
	const bool discard;
	const ClearValueConfig clearValue;

	AttachmentConfig(const YAML::Node &node);
};

std::unordered_map<std::string, AttachmentConfig> parseAttachmentConfigs(const YAML::Node &node);

} // namespace config
