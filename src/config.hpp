//! orge初期化用のyamlをパースするモジュール

#pragma once

#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

struct Attachment {
	vk::Format format;
	bool discard;
	vk::ImageLayout finalLayout;
	vk::ClearValue clearValue;
};

struct Subpass {
	std::vector<vk::AttachmentReference> inputs;
	std::vector<vk::AttachmentReference> outputs;
	std::optional<vk::AttachmentReference> depth;
};

struct SubpassDependency {
	uint32_t src;
	uint32_t dst;
};

struct Pipeline {
	std::vector<std::vector<vk::DescriptorSetLayoutBinding>> descSets;
	std::string vertexShader;
	std::string fragmentShader;
	std::vector<uint32_t> vertexInputAttributes;
	bool culling;
	bool colorBlend;
};

struct Config {
	std::string title;
	int width;
	int height;
	std::vector<Attachment> attachments;
	std::vector<Subpass> subpasses;
	std::vector<SubpassDependency> subpassDeps;
	std::vector<Pipeline> pipelines;
};

std::optional<Config> parseConfig(const char *const yaml);

std::optional<Config> parseConfigFromFile(const char *const yamlFilePath);
