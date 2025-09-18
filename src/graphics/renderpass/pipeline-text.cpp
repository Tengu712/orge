#include "pipeline-text.hpp"

#include "../../config/config.hpp"
#include "../core/core.hpp"
#include "../resource/descpool.hpp"
#include "pipeline-utils.hpp"

#include <text-shader-frag-spv.cpp>
#include <text-shader-vert-spv.cpp>

namespace graphics::renderpass {

void createTextRenderingPipeline(
	const vk::RenderPass &renderPass,
	uint32_t subpassIndex,
	std::unordered_map<uint32_t, GraphicsPipeline> &pipelines
) {
	const auto &device = core::device();

	// シェーダステージ
	const auto vs = createShaderModule(text_shader_vert_spv, text_shader_vert_spv_len);
	const auto fs = createShaderModule(text_shader_frag_spv, text_shader_frag_spv_len);
	const auto fspme = vk::SpecializationMapEntry(0, 0, sizeof(uint32_t));
	const auto fontCount = config::config().fonts.size();
	const auto fspi = vk::SpecializationInfo(1, &fspme, sizeof(uint32_t), &fontCount);
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
	shaderStages.reserve(2);
	shaderStages.emplace_back(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex,   vs.get(), "main");
	shaderStages.emplace_back(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, fs.get(), "main", &fspi);

	// 頂点入力
	std::vector<vk::VertexInputAttributeDescription> viads;
	std::vector<vk::VertexInputBindingDescription> vibds;
	const auto vertexInputState = vk::PipelineVertexInputStateCreateInfo()
		.setVertexBindingDescriptions(vibds)
		.setVertexAttributeDescriptions(viads);

	// 頂点アセンブリ
	const auto inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo()
		.setTopology(vk::PrimitiveTopology::eTriangleStrip);

	// ラスタライゼーション
	const auto rasterizationState = vk::PipelineRasterizationStateCreateInfo()
		.setPolygonMode(vk::PolygonMode::eFill)
		.setCullMode(vk::CullModeFlagBits::eNone)
		.setFrontFace(vk::FrontFace::eCounterClockwise)
		.setLineWidth(1.0f);

	// デプスステンシル
	const auto depthStencilState = vk::PipelineDepthStencilStateCreateInfo()
		.setDepthTestEnable(false)
		.setDepthWriteEnable(false)
		.setDepthCompareOp(vk::CompareOp::eLess)
		.setMaxDepthBounds(1.0f);

	// カラーブレンド
	std::vector<vk::PipelineColorBlendAttachmentState> cbass;
	cbass.reserve(1);
	cbass.emplace_back(
		vk::True,
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
	const auto colorBlendState = vk::PipelineColorBlendStateCreateInfo()
		.setAttachments(cbass);

	// ディスクリプタセットレイアウト
	std::vector<vk::UniqueDescriptorSetLayout> descSetLayouts;
	descSetLayouts.reserve(2);
	std::vector<vk::DescriptorSetLayoutBinding> bindings;
	bindings.reserve(2);
	bindings.emplace_back(
		0,
		vk::DescriptorType::eStorageBuffer,
		1,
		vk::ShaderStageFlagBits::eVertex,
		nullptr
	);
	descSetLayouts.push_back(device.createDescriptorSetLayoutUnique(
		vk::DescriptorSetLayoutCreateInfo().setBindings(bindings)
	));
	bindings.clear();
	bindings.emplace_back(
		0,
		vk::DescriptorType::eSampledImage,
		static_cast<uint32_t>(fontCount),
		vk::ShaderStageFlagBits::eFragment,
		nullptr
	);
	bindings.emplace_back(
		1,
		vk::DescriptorType::eSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		nullptr
	);
	descSetLayouts.push_back(device.createDescriptorSetLayoutUnique(
		vk::DescriptorSetLayoutCreateInfo().setBindings(bindings)
	));

	// ディスクリプタセット確保
	std::vector<std::vector<vk::UniqueDescriptorSet>> descSetss;
	descSetss.reserve(2);
	std::vector<vk::DescriptorSetLayout> layouts;
	layouts.reserve(1);
	layouts.push_back(descSetLayouts[0].get());
	descSetss.push_back(device.allocateDescriptorSetsUnique(
		vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(resource::descpool())
			.setSetLayouts(layouts)
	));
	layouts.clear();
	layouts.push_back(descSetLayouts[1].get());
	descSetss.push_back(device.allocateDescriptorSetsUnique(
		vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(resource::descpool())
			.setSetLayouts(layouts)
	));

	// パイプラインレイアウト
	std::vector<vk::DescriptorSetLayout> rawDescSetLayouts;
	for (const auto& m: descSetLayouts) {
		rawDescSetLayouts.push_back(m.get());
	}
	const auto plci = vk::PipelineLayoutCreateInfo()
		.setSetLayouts(rawDescSetLayouts);
	auto pipelineLayout = device.createPipelineLayoutUnique(plci);

	// 他
	const auto viewportState = createPipelineViewStateCreateInfo();
	const auto multisampleState = createPipelineMultisampleStateCreateInfo();

	// 作成
	std::vector<vk::GraphicsPipelineCreateInfo> cis;
	cis.reserve(1);
	cis.push_back(
		vk::GraphicsPipelineCreateInfo()
			.setStages(shaderStages)
			.setPVertexInputState(&vertexInputState)
			.setPInputAssemblyState(&inputAssemblyState)
			.setPViewportState(&viewportState.ci)
			.setPRasterizationState(&rasterizationState)
			.setPMultisampleState(&multisampleState)
			.setPDepthStencilState(&depthStencilState)
			.setPColorBlendState(&colorBlendState)
			.setLayout(pipelineLayout.get())
			.setRenderPass(renderPass)
			.setSubpass(subpassIndex)
	);
	auto createds = device.createGraphicsPipelinesUnique(nullptr, cis).value;
	if (createds.empty()) {
		throw "failed to create a text rendering pipeline.";
	}

	pipelines.try_emplace(
		subpassIndex,
		"@text@",
		std::move(createds[0]),
		std::move(pipelineLayout),
		std::move(descSetLayouts),
		std::move(descSetss)
	);
}

} // namespace graphics::renderpass
