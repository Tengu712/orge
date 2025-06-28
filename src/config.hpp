//! orge初期化用のyamlをパースするモジュール

#pragma once

#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

struct Attachment {
	const vk::Format format;
	const bool discard;
	const vk::ImageLayout finalLayout;
	const vk::ClearValue clearValue;
};

struct Subpass {
	const std::vector<vk::AttachmentReference> inputs;
	const std::vector<vk::AttachmentReference> outputs;
	const std::optional<vk::AttachmentReference> depth;
};

struct SubpassDependency {
	const uint32_t src;
	const uint32_t dst;
};

struct Config {
	const std::string title;
	const int width;
	const int height;
	const std::vector<Attachment> attachments;
	const std::vector<Subpass> subpasses;
	const std::vector<SubpassDependency> subpassDeps;
};

std::optional<Config> parseConfig(const char *const yaml);

std::optional<Config> parseConfigFromFile(const char *const yamlFilePath);
