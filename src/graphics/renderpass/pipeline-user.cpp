#include "pipeline-user.hpp"

#include "../../asset/asset.hpp"
#include "../../config/config.hpp"
#include "../../config/enumconvert.hpp"
#include "../../error/error.hpp"
#include "../core/core.hpp"
#include "../resource/descpool.hpp"
#include "pipeline-utils.hpp"

namespace graphics::renderpass {

void createGraphicsPipeline(
	const vk::RenderPass &renderPass,
	const std::string &pipelineId,
	uint32_t subpassIndex,
	std::unordered_map<std::string, GraphicsPipeline> &pipelines
) {
	const auto &n = config::config().pipelines.at(pipelineId);
	const auto &device = core::device();

	// シェーダステージ
	const auto vsAId = error::at(config::config().assetMap, n.vertexShader,   "assets");
	const auto fsAId = error::at(config::config().assetMap, n.fragmentShader, "assets");
	const auto vsRaw = asset::getAsset(vsAId);
	const auto fsRaw = asset::getAsset(fsAId);
	const auto vs = createShaderModule(vsRaw.data(), vsRaw.size());
	const auto fs = createShaderModule(fsRaw.data(), fsRaw.size());
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
	shaderStages.reserve(2);
	shaderStages.emplace_back(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex,   vs, "main");
	shaderStages.emplace_back(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, fs, "main");

	// 頂点入力
	std::vector<vk::VertexInputAttributeDescription> viads;
	viads.reserve(n.vertexInputAttributes.size());
	uint32_t sum = 0;
	for (size_t i = 0; i < n.vertexInputAttributes.size(); ++i) {
		const auto &m = n.vertexInputAttributes[i];
		viads.emplace_back(
			static_cast<uint32_t>(i),
			0,
			m == 1 ? vk::Format::eR32Sfloat
			: m == 2 ? vk::Format::eR32G32Sfloat
			: m == 3 ? vk::Format::eR32G32B32Sfloat
			: vk::Format::eR32G32B32A32Sfloat,
			static_cast<uint32_t>(sizeof(float)) * sum
		);
		sum += m;
	}
	std::vector<vk::VertexInputBindingDescription> vibds;
	vibds.reserve(1);
	vibds.emplace_back(0, static_cast<uint32_t>(sizeof(float) * sum), vk::VertexInputRate::eVertex);
	const auto vertexInputState = vk::PipelineVertexInputStateCreateInfo()
		.setVertexBindingDescriptions(vibds)
		.setVertexAttributeDescriptions(viads);

	// 頂点アセンブリ
	const auto inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo()
		.setTopology(n.meshInShader ? vk::PrimitiveTopology::eTriangleStrip : vk::PrimitiveTopology::eTriangleList);

	// ラスタライゼーション
	const auto rasterizationState = vk::PipelineRasterizationStateCreateInfo()
		.setPolygonMode(vk::PolygonMode::eFill)
		.setCullMode(n.culling ? vk::CullModeFlagBits::eBack : vk::CullModeFlagBits::eNone)
		.setFrontFace(vk::FrontFace::eCounterClockwise)
		.setLineWidth(1.0f);

	// デプスステンシル
	const auto depthStencilState = vk::PipelineDepthStencilStateCreateInfo()
		.setDepthTestEnable(n.depthTest)
		.setDepthWriteEnable(n.depthTest)
		.setDepthCompareOp(vk::CompareOp::eLess)
		.setMaxDepthBounds(1.0f);

	// カラーブレンド
	std::vector<vk::PipelineColorBlendAttachmentState> cbass;
	cbass.reserve(n.colorBlends.size());
	for (const auto &m: n.colorBlends) {
		cbass.emplace_back(
			m ? vk::True : vk::False,
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
	const auto colorBlendState = vk::PipelineColorBlendStateCreateInfo()
		.setAttachments(cbass);

	// ディスクリプタセットレイアウト
	std::vector<vk::DescriptorSetLayout> descSetLayouts;
	descSetLayouts.reserve(n.descSets.size());
	for (const auto &m: n.descSets) {
		std::vector<vk::DescriptorSetLayoutBinding> bindings;
		for (const auto &o: m.bindings) {
			bindings.emplace_back(
				static_cast<uint32_t>(bindings.size()),
				config::convertDescriptorType(o.type),
				o.count,
				config::convertShaderStages(o.stage),
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
	const auto pipelineLayout = device.createPipelineLayout(plci);

	// ディスクリプタセット確保
	std::vector<std::vector<vk::DescriptorSet>> descSets;
	descSets.reserve(n.descSets.size());
	for (size_t i = 0; i < n.descSets.size(); ++i) {
		std::vector<vk::DescriptorSetLayout> layouts;
		for (size_t j = 0; j < n.descSets[i].count; ++j) {
			layouts.push_back(descSetLayouts[i]);
		}
		const auto ai = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(resource::descpool())
			.setSetLayouts(layouts);
		descSets.push_back(device.allocateDescriptorSets(ai));
	}

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
			.setLayout(pipelineLayout)
			.setRenderPass(renderPass)
			.setSubpass(subpassIndex)
	);
	const auto createds = device.createGraphicsPipelines(nullptr, cis).value;
	if (createds.empty()) {
		throw std::format("failed to create a pipeline, '{}'.", pipelineId);
	}

	core::device().destroy(vs);
	core::device().destroy(fs);

	pipelines.try_emplace(
		pipelineId,
		pipelineId,
		createds[0],
		pipelineLayout,
		std::move(descSetLayouts),
		std::move(descSets)
	);
}

} // namespace graphics::renderpass
