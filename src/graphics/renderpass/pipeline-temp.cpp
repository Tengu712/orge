#include "pipeline-temp.hpp"

#include "../../asset/asset.hpp"
#include "../../config/config.hpp"
#include "../../config/enumconvert.hpp"
#include "../../error/error.hpp"
#include "../core/core.hpp"
#include "../resource/descpool.hpp"

#include <text-shader-frag-spv.cpp>
#include <text-shader-vert-spv.cpp>

namespace graphics::renderpass {

vk::ShaderModule createShaderModule(const unsigned char *data, size_t length) {
	const auto code = std::vector<uint32_t>(
		reinterpret_cast<const uint32_t *>(data),
		reinterpret_cast<const uint32_t *>(data + length)
	);
	const auto ci = vk::ShaderModuleCreateInfo()
		.setCode(code);
	return core::device().createShaderModule(ci);
}

PipelineCreateTemporaryInfos::PipelineCreateTemporaryInfos(
	const std::string &pipelineId,
	uint32_t subpassIndex
):
	id(pipelineId), subpass(subpassIndex)
{
	const auto &n = config::config().pipelines.at(pipelineId);
	const auto &device = core::device();

	// シェーダステージ
	if (n.textRendering) {
		vertexShader   = createShaderModule(text_shader_vert_spv, text_shader_vert_spv_len);
		fragmentShader = createShaderModule(text_shader_frag_spv, text_shader_frag_spv_len);
		texCount = static_cast<uint32_t>(config::config().fonts.size());
		fspme = vk::SpecializationMapEntry(0, 0, sizeof(uint32_t));
		fspi = vk::SpecializationInfo(1, &fspme, sizeof(uint32_t), &texCount);
	} else {
		const auto vsAId = error::at(config::config().assetMap, n.vertexShader,   "assets");
		const auto fsAId = error::at(config::config().assetMap, n.fragmentShader, "assets");
		const auto vs = asset::getAsset(vsAId);
		const auto fs = asset::getAsset(fsAId);
		vertexShader   = createShaderModule(vs.data(), vs.size());
		fragmentShader = createShaderModule(fs.data(), fs.size());
	}
	if (!vertexShader || !fragmentShader) {
		throw "failed to create shader modules.";
	}
	sscis.emplace_back(
		vk::PipelineShaderStageCreateFlags(),
		vk::ShaderStageFlagBits::eVertex,
		vertexShader,
		"main"
	);
	sscis.emplace_back(
		vk::PipelineShaderStageCreateFlags(),
		vk::ShaderStageFlagBits::eFragment,
		fragmentShader,
		"main",
		&fspi
	);

	// 頂点入力
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
	vibds.emplace_back(0, static_cast<uint32_t>(sizeof(float) * sum), vk::VertexInputRate::eVertex);
	visci = vk::PipelineVertexInputStateCreateInfo()
		.setVertexBindingDescriptions(vibds)
		.setVertexAttributeDescriptions(viads);

	// 頂点アセンブリ
	iasci = vk::PipelineInputAssemblyStateCreateInfo()
		.setTopology(n.textRendering ? vk::PrimitiveTopology::eTriangleStrip : vk::PrimitiveTopology::eTriangleList);

	// ラスタライゼーション
	rsci = vk::PipelineRasterizationStateCreateInfo()
		.setPolygonMode(vk::PolygonMode::eFill)
		.setCullMode(n.culling ? vk::CullModeFlagBits::eBack : vk::CullModeFlagBits::eNone)
		.setFrontFace(vk::FrontFace::eCounterClockwise)
		.setLineWidth(1.0f);

	// デプスステンシル
	dssci = vk::PipelineDepthStencilStateCreateInfo()
		.setDepthTestEnable(n.depthTest)
		.setDepthWriteEnable(n.depthTest)
		.setDepthCompareOp(vk::CompareOp::eLess)
		.setMaxDepthBounds(1.0f);

	// カラーブレンド
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
	cbsci = vk::PipelineColorBlendStateCreateInfo()
		.setAttachments(cbass);

	// ディスクリプタセットレイアウト
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
	pipelineLayout = device.createPipelineLayout(plci);

	// ディスクリプタセット確保
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
}

PipelineCreateTemporaryInfos::~PipelineCreateTemporaryInfos() {
	core::device().destroy(vertexShader);
	core::device().destroy(fragmentShader);
}

} // namespace graphics::renderpass
