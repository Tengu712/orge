#pragma once

#include "attachment.hpp"
#include "compute.hpp"
#include "font.hpp"
#include "mesh.hpp"
#include "pipeline.hpp"
#include "renderpass.hpp"

namespace config {

struct Config {
	const std::string title;
	const uint32_t width;
	const uint32_t height;
	const bool fullscreen;
	const bool disableVsync;
	const bool altReturnToggleFullscreen;
	const uint32_t audioChannelCount;
	const uint32_t charCount;
	const std::unordered_map<std::string, MeshConfig> meshes;
	const std::unordered_map<std::string, FontConfig> fonts;
	const std::unordered_map<std::string, AttachmentConfig> attachments;
	const std::unordered_map<std::string, PipelineConfig> pipelines;
	const std::unordered_map<std::string, RenderPassConfig> renderPasses;
	const std::unordered_map<std::string, ComputePipelineConfig> computePipelines;

	const std::unordered_map<std::string, uint32_t> assetMap;
	const std::unordered_map<std::string, uint32_t> fontMap;

	Config(const YAML::Node &node);
};

void initialize();

const Config &config();

} // namespace config
