#pragma once

#include "config/pipeline.hpp"
#include "config/subpass.hpp"

namespace config {

struct Config {
	std::string title;
	uint32_t width;
	uint32_t height;
	std::vector<vk::AttachmentDescription> attachments;
	std::vector<vk::ClearValue> clearValues;
	std::vector<SubpassConfig> subpasses;
	std::vector<vk::SubpassDependency> dependencies;
	std::vector<PipelineConfig> pipelines;
};

Config parse(const char *const yaml);

Config parseFromFile(const char *const yamlFilePath);

} // namespace config
