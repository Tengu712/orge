#include "config.hpp"

#include <fstream>

namespace graphics::pipeline {

vk::ShaderModule createShaderModule(const vk::Device &device, const std::string &path) {
	std::fstream file(path, std::ios::in | std::ios::binary);
	if (!file) {
		return nullptr;
	}

	file.seekg(0, std::ios::end);
	const auto size = file.tellg();
	file.seekg(0, std::ios::beg);

	if (size <= 0 || size % sizeof(uint32_t) != 0) {
		return nullptr;
	}

	std::vector<uint32_t> code(size / sizeof(uint32_t));
	file.read(reinterpret_cast<char *>(code.data()), size);

	if (!file || file.gcount() != size) {
		return nullptr;
	}

	const auto ci = vk::ShaderModuleCreateInfo()
		.setCode(code);
	return device.createShaderModule(ci);
}

PipelineCreateDynamicInfo::PipelineCreateDynamicInfo(
	const config::PipelineConfig &config,
	const std::unordered_map<std::string, uint32_t> &subpassMap,
	const vk::Device &device,
	const vk::DescriptorPool &descPool
) {
	// シェーダステージ
	vertexShader = createShaderModule(device, config.vertexShader);
	fragmentShader = createShaderModule(device, config.fragmentShader);
	if (!vertexShader || !fragmentShader) {
		throw "failed to create shader modules.";
	}
	psscis.emplace_back(
		vk::PipelineShaderStageCreateFlags(),
		vk::ShaderStageFlagBits::eVertex,
		vertexShader,
		"main"
	);
	psscis.emplace_back(
		vk::PipelineShaderStageCreateFlags(),
		vk::ShaderStageFlagBits::eFragment,
		fragmentShader,
		"main"
	);

	// 頂点入力
	uint32_t sum = 0;
	for (size_t i = 0; i < config.vertexInputAttributes.size(); ++i) {
		const auto &n = config.vertexInputAttributes.at(i);
		viads.emplace_back(
			static_cast<uint32_t>(i),
			0,
			n == 1 ? vk::Format::eR32Sfloat
			: n == 2 ? vk::Format::eR32G32Sfloat
			: n == 3 ? vk::Format::eR32G32B32Sfloat
			: vk::Format::eR32G32B32A32Sfloat,
			static_cast<uint32_t>(sizeof(float)) * sum
		);
		sum += n;
	}
	vibds.emplace_back(0, sizeof(float) * sum, vk::VertexInputRate::eVertex);
	pvisci = vk::PipelineVertexInputStateCreateInfo()
		.setVertexBindingDescriptions(vibds)
		.setVertexAttributeDescriptions(viads);

	// ラスタライゼーション
	prsci = vk::PipelineRasterizationStateCreateInfo()
		.setPolygonMode(vk::PolygonMode::eFill)
		.setCullMode(config.culling ? vk::CullModeFlagBits::eBack : vk::CullModeFlagBits::eNone)
		.setFrontFace(vk::FrontFace::eCounterClockwise)
		.setLineWidth(1.0f);

	// カラーブレンド
	for (const auto &n: config.colorBlends) {
		pcbass.emplace_back(
			n ? vk::True : vk::False,
			vk::BlendFactor::eSrcAlpha,
			vk::BlendFactor::eOneMinusSrcAlpha,
			vk::BlendOp::eAdd,
			vk::BlendFactor::eSrcAlpha,
			vk::BlendFactor::eOneMinusSrcAlpha,
			vk::BlendOp::eAdd,
			vk::ColorComponentFlagBits::eR
			| vk::ColorComponentFlagBits::eG
			| vk::ColorComponentFlagBits::eB
			| vk::ColorComponentFlagBits::eA
		);
	}
	pcbsci = vk::PipelineColorBlendStateCreateInfo()
		.setAttachments(pcbass);

	// ディスクリプタセットレイアウト
	for (const auto &n: config.descSets) {
		std::vector<vk::DescriptorSetLayoutBinding> bindings;
		for (const auto &m: n.bindings) {
			bindings.emplace_back(
				static_cast<uint32_t>(bindings.size()),
				m.type == config::DescriptorType::CombinedImageSampler
					? vk::DescriptorType::eCombinedImageSampler
					: m.type == config::DescriptorType::UniformBuffer
					? vk::DescriptorType::eUniformBuffer
					: m.type == config::DescriptorType::StorageBuffer
					? vk::DescriptorType::eStorageBuffer
					: m.type == config::DescriptorType::InputAttachment
					? vk::DescriptorType::eInputAttachment
					: throw,
				m.count,
				m.stages == config::ShaderStages::Vertex
					? vk::ShaderStageFlagBits::eVertex
					: m.stages == config::ShaderStages::Fragment
					? vk::ShaderStageFlagBits::eFragment
					: m.stages == config::ShaderStages::VertexAndFragment
					? vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment
					: throw,
				nullptr
			);
		}
		const auto ci = vk::DescriptorSetLayoutCreateInfo()
			.setBindings(bindings);
		descSetLayouts.push_back(device.createDescriptorSetLayout(ci));
	}

	// パイプラインレイアウト
	const auto plci = vk::PipelineLayoutCreateInfo()
		.setSetLayouts(descSetLayouts);
	pipelineLayout = device.createPipelineLayout(plci);

	// サブパスID
	if (!subpassMap.contains(config.subpass)) {
		throw std::format("subpass '{}' is not defined.", config.subpass);
	}
	subpass = subpassMap.at(config.subpass);

	// ディスクリプタセット確保
	descSets.reserve(config.descSets.size());
	for (size_t i = 0; i < config.descSets.size(); ++i) {
		std::vector<vk::DescriptorSetLayout> layouts;
		for (size_t j = 0; j < config.descSets[i].count; ++j) {
			layouts.push_back(descSetLayouts[j]);
		}
		const auto ai = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(descPool)
			.setSetLayouts(layouts);
		descSets.push_back(device.allocateDescriptorSets(ai));
	}
}

void PipelineCreateDynamicInfo::destroy(const vk::Device &device) {
	descSets.clear();
	descSetLayouts.clear();

	device.destroyShaderModule(fragmentShader);
	device.destroyShaderModule(vertexShader);
}

} // namespace graphics::pipeline
