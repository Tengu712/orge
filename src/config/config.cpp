#include "config.hpp"

#include <format>

namespace config {

std::unordered_map<std::string, FontConfig> createFonts(OrgeInitializeParam *param) {
	std::unordered_map<std::string, FontConfig> fonts;
	fonts.reserve(param->fontCount);
	for (uint32_t i = 0; i < param->fontCount; ++i) {
		const auto &font = param->fonts[i];
		fonts.try_emplace(font.id, font.file, font.charSize, font.charAtlusCol, font.charAtlusRow);
	}
	return fonts;
}

std::unordered_map<std::string, AttachmentConfig> createAttachments(OrgeInitializeParam *param) {
	std::unordered_map<std::string, AttachmentConfig> attachments;
	attachments.reserve(param->attachmentCount);
	for (uint32_t i = 0; i < param->attachmentCount; ++i) {
		const auto &n = param->attachments[i];

		ClearValueConfig clearValue;
		switch (n.clearValueType) {
		case ORGE_CLEAR_VALUE_TYPE_COLOR:
			clearValue = ColorClearValue{n.clearValue[0], n.clearValue[1], n.clearValue[2], n.clearValue[3]};
			break;
		case ORGE_CLEAR_VALUE_TYPE_DEPTH:
			clearValue = DepthClearValue{n.clearValue[0]};
			break;
		default:
			throw std::format("invalid clear value type passed: {}", static_cast<uint32_t>(n.clearValueType));
		}

		Format format;
		switch (n.format) {
		case ORGE_FORMAT_RENDER_TARGET:
			format = Format::RenderTarget;
			break;
		case ORGE_FORMAT_DEPTH_BUFFER:
			format = Format::DepthBuffer;
			break;
		case ORGE_FORMAT_SHARE_COLOR_ATTACHMENT:
			format = Format::ShareColorAttachment;
			break;
		case ORGE_FORMAT_SIGNED_SHARE_COLOR_ATTACHMENT:
			format = Format::SignedShareColorAttachment;
			break;
		default:
			throw std::format("invalid attachment format passed: {}", static_cast<uint32_t>(n.format));
		}

		attachments.try_emplace(n.id, format, static_cast<bool>(n.discard), clearValue);
	}
	return attachments;
}

std::vector<DescriptorBindingConfig> createDescriptorBindings(
	uint32_t bindingCount,
	OrgeDescriptorBindingConfig *bindings
) {
	std::vector<DescriptorBindingConfig> result;
	result.reserve(bindingCount);
	for (uint32_t i = 0; i < bindingCount; ++i) {
		const auto& n = bindings[i];

		DescriptorType type;
		switch (n.type) {
		case ORGE_DESCRIPTOR_TYPE_TEXTURE:
			type = DescriptorType::Texture;
			break;
		case ORGE_DESCRIPTOR_TYPE_SAMPLER:
			type = DescriptorType::Sampler;
			break;
		case ORGE_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			type = DescriptorType::UniformBuffer;
			break;
		case ORGE_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			type = DescriptorType::StorageBuffer;
			break;;
		case ORGE_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
			break;
		default:
			throw std::format("invalid descriptor binding type passed: {}", static_cast<uint32_t>(n.type));
		}

		ShaderStages stage;
		switch (n.stage) {
		case ORGE_SHADER_STAGES_VERTEX:
			stage = ShaderStages::Vertex;
			break;
		case ORGE_SHADER_STAGES_FRAGMENT:
			stage = ShaderStages::Fragment;
			break;
		case ORGE_SHADER_STAGES_VERTEX_AND_FRAGMENT:
			stage = ShaderStages::VertexAndFragment;
			break;
		}

		result.emplace_back(type, n.count, stage);
	}
	return result;
}

std::vector<DescriptorSetConfig> createDescriptorSets(uint32_t descSetCount, const OrgeDescriptorSetConfig *descSets) {
	std::vector<DescriptorSetConfig> result;
	result.reserve(descSetCount);
	for (uint32_t i = 0; i < descSetCount; ++i) {
		const auto &n = descSets[i];
		auto bindings = createDescriptorBindings(n.bindingCount, n.bindings);
		result.emplace_back(n.count, std::move(bindings));
	}
	return result;
}

std::unordered_map<std::string, GraphicsPipelineConfig> createPipelines(OrgeInitializeParam *param) {
	std::unordered_map<std::string, GraphicsPipelineConfig> result;
	result.reserve(static_cast<size_t>(param->graphicsPipelineCount));
	for (uint32_t i = 0; i < param->graphicsPipelineCount; ++i) {
		const auto &n = param->graphicsPipelines[i];

		auto descSets = createDescriptorSets(n.descSetCount, n.descSets);

		std::vector<uint32_t> vertexInputAttributes;
		vertexInputAttributes.reserve(n.vertexInputAttributeCount);
		for (uint32_t j = 0; j < n.vertexInputAttributeCount; ++j) {
			vertexInputAttributes.push_back(n.vertexInputAttributes[j]);
		}

		std::vector<bool> colorBlends;
		colorBlends.reserve(n.colorBlendCount);
		for (uint32_t j = 0; j < n.colorBlendCount; ++j) {
			colorBlends.push_back(n.colorBlends[j]);
		}

		result.try_emplace(
			n.id,
			n.vertexShaderFile,
			n.fragmentShaderFile,
			std::move(descSets),
			std::move(vertexInputAttributes),
			n.meshInShader,
			n.culling,
			n.depthTest,
			std::move(colorBlends)
		);
	}
	return result;
}

std::unordered_set<std::string> createStringSet(uint32_t count, const char **strings) {
	std::unordered_set<std::string> result;
	result.reserve(static_cast<size_t>(count));
	for (uint32_t i = 0; i < count; ++i) {
		result.insert(strings[i]);
	}
	return result;
}

std::vector<SubpassConfig> createSubpasses(uint32_t subpassCount, const OrgeSubpassConfig *subpasses) {
	std::vector<SubpassConfig> result;
	result.reserve(static_cast<size_t>(subpassCount));
	for (uint32_t i = 0; i < subpassCount; ++i) {
		const auto &n = subpasses[i];

		auto inputs =  createStringSet(n.inputCount,  n.inputs);
		auto outputs = createStringSet(n.outputCount, n.outputs);

		std::optional<SubpassDepthConfig> depth;
		if (n.depth) {
			depth.emplace(n.depth->depth, n.depth->readOnly);
		}

		auto depends =   createStringSet(n.dependCount,   n.depends);
		auto pipelines = createStringSet(n.pipelineCount, n.pipelines);

		result.emplace_back(
			n.id,
			std::move(inputs),
			std::move(outputs),
			std::move(depth),
			std::move(depends),
			std::move(pipelines)
		);
	}
	return result;
}

std::unordered_map<std::string, RenderPassConfig> createRenderPasses(OrgeInitializeParam *param) {
	std::unordered_map<std::string, RenderPassConfig> result;
	result.reserve(param->renderPassCount);
	for (uint32_t i = 0; i < param->renderPassCount; ++i) {
		const auto &n = param->renderPasses[i];

		auto subpasses = createSubpasses(n.subpassCount, n.subpasses);

		std::vector<std::string> attachments;
		std::unordered_set<std::string> attachmentSet;
		for (const auto &m: subpasses) {
			for (const auto &o: m.inputs) {
				if (!attachmentSet.contains(o)) {
					attachments.push_back(o);
					attachmentSet.insert(o);
				}
			}
			for (const auto &o: m.outputs) {
				if (!attachmentSet.contains(o)) {
					attachments.push_back(o);
					attachmentSet.insert(o);
				}
			}
			if (m.depth.has_value()) {
				const auto &o = m.depth.value();
				if (!attachmentSet.contains(o.id)) {
					attachments.push_back(o.id);
					attachmentSet.insert(o.id);
				}
			}
		}

		std::unordered_map<std::string, uint32_t> attachmentMap;
		attachmentMap.reserve(attachments.size());
		for (uint32_t j = 0; j < attachments.size(); ++j) {
			attachmentMap.emplace(param->attachments[j].id, j);
		}

		std::unordered_map<std::string, uint32_t> subpassMap;
		subpassMap.reserve(subpasses.size());
		for (uint32_t j = 0; j < subpasses.size(); ++j) {
			subpassMap.emplace(subpasses[j].id, j);
		}

		result.try_emplace(
			n.id,
			std::move(subpasses),
			std::move(attachments),
			std::move(attachmentMap),
			std::move(subpassMap)
		);
	}
	return result;
}

std::vector<ComputeDescriptorBindingConfig> createComputeDescriptorBindings(
	uint32_t bindingCount,
	const OrgeComputeDescriptorBindingConfig *bindings
) {
	std::vector<ComputeDescriptorBindingConfig> result;
	result.reserve(bindingCount);
	for (uint32_t i = 0; i < bindingCount; ++i) {
		const auto &n = bindings[i];

		ComputeDescriptorType type;
		switch (n.type) {
		case ORGE_COMPUTE_DESCRIPTOR_TYPE_TEXTURE:
			type = ComputeDescriptorType::Texture;
			break;
		case ORGE_COMPUTE_DESCRIPTOR_TYPE_SAMPLER:
			type = ComputeDescriptorType::Sampler;
			break;
		case ORGE_COMPUTE_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			type = ComputeDescriptorType::UniformBuffer;
			break;
		case ORGE_COMPUTE_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			type = ComputeDescriptorType::StorageBuffer;
			break;
		case ORGE_COMPUTE_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			type = ComputeDescriptorType::StorageImage;
			break;
		default:
			throw std::format("invalid compute descriptor binding type passed: {}", static_cast<uint32_t>(n.type));
		}

		result.emplace_back(type, n.count);
	}
	return result;
}

std::vector<ComputeDescriptorSetConfig> createComputeDescriptorSets(
	uint32_t descSetCount,
	const OrgeComputeDescriptorSetConfig *descSets
) {
	std::vector<ComputeDescriptorSetConfig> result;
	result.reserve(descSetCount);
	for (uint32_t i = 0; i < descSetCount; ++i) {
		const auto &n = descSets[i];
		auto bindings = createComputeDescriptorBindings(n.bindingCount, n.bindings);
		result.emplace_back(n.count, std::move(bindings));
	}
	return result;
}

std::unordered_map<std::string, ComputePipelineConfig> createComputePipelines(OrgeInitializeParam *param) {
	std::unordered_map<std::string, ComputePipelineConfig> result;
	result.reserve(param->computePipelineCount);
	for (uint32_t i = 0; i < param->computePipelineCount; ++i) {
		const auto& n = param->computePipelines[i];
		auto descSets = createComputeDescriptorSets(n.descSetCount, n.descSets);
		result.try_emplace(
			n.id,
			n.shaderFile,
			std::move(descSets)
		);
	}
	return result;
}

std::optional<Config> g_config;

void initialize(OrgeInitializeParam *param) {
	auto fonts = createFonts(param);
	auto attachments = createAttachments(param);
	auto pipelines = createPipelines(param);
	auto renderPasses = createRenderPasses(param);
	auto computePipelines = createComputePipelines(param);
	std::unordered_map<std::string, uint32_t> assetMap;
	assetMap.reserve(param->assetCount);
	for (uint32_t i = 0; i < param->assetCount; ++i) {
		assetMap.emplace(param->assets[i], i);
	}
	std::unordered_map<std::string, uint32_t> fontMap;
	fontMap.reserve(param->fontCount);
	for (uint32_t i = 0; i < param->fontCount; ++i) {
		fontMap.emplace(param->fonts[i].id, i);
	}

	g_config.emplace(
		param->title,
		param->width,
		param->height,
		static_cast<bool>(param->fullscreen),
		static_cast<bool>(param->disableFullscreenShortcut),
		static_cast<bool>(param->disableVsync),
		param->audioChannelCount,
		param->charCount,
		std::move(fonts),
		std::move(attachments),
		std::move(pipelines),
		std::move(renderPasses),
		std::move(computePipelines),
		std::move(assetMap),
		std::move(fontMap)
	);
}

const Config &config() {
	if (g_config.has_value()) {
		return g_config.value();
	} else {
		throw "config not initialized.";
	}
}

} // namespace config
