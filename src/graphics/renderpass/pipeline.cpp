#include "pipeline.hpp"

#include "../../config/config.hpp"
#include "../core/core.hpp"
#include "../resource/buffer.hpp"
#include "../resource/image-attachment.hpp"
#include "../resource/image-user.hpp"
#include "../resource/sampler.hpp"
#include "../window/swapchain.hpp"
#include "pipeline-temp.hpp"

namespace graphics::renderpass {

Pipeline::~Pipeline() {
	for (const auto &n: _descSetLayouts) {
		core::device().destroyDescriptorSetLayout(n);
	}
	core::device().destroyPipelineLayout(_pipelineLayout);
	core::device().destroyPipeline(_pipeline);
}

void Pipeline::updateInputAttachmentDescriptor(
	const std::string &id,
	uint32_t set,
	uint32_t index,
	uint32_t binding,
	uint32_t offset,
	uint32_t frameIndex
) const {
	const auto &descSets = error::at(_descSets, set, "descriptor sets");
	const auto &descSet = error::at(descSets, index, "descriptor sets allocated");
	const auto &cdescSets = error::at(config::config().pipelines.at(_id).descSets, set, "descriptor sets");
	const auto &cdescSet = error::at(cdescSets.bindings, binding, "descriptor set bindings");
	const auto isInputAttachment =
		cdescSet.type == config::DescriptorType::InputAttachment ? true
		: cdescSet.type == config::DescriptorType::Texture ? false
		: throw std::format("input attachment must be bound as input-attachment or texture: set={}, binding={}", set, binding);
	const auto &image = resource::getAttachmentImage(frameIndex, id);
	const auto ii = vk::DescriptorImageInfo(nullptr, image.get(), vk::ImageLayout::eShaderReadOnlyOptimal);
	const auto ds = vk::WriteDescriptorSet()
		.setDstSet(descSet)
		.setDstBinding(binding)
		.setDstArrayElement(offset)
		.setDescriptorCount(1)
		.setDescriptorType(isInputAttachment ? vk::DescriptorType::eInputAttachment : vk::DescriptorType::eSampledImage)
		.setImageInfo(ii);
	core::device().updateDescriptorSets(1, &ds, 0, nullptr);
}

#define DECLARE_UPDATE_DESC_METHOD(n) \
	void Pipeline::n( \
		const std::string &id, \
		uint32_t set, \
		uint32_t index, \
		uint32_t binding, \
		uint32_t offset \
	) const

DECLARE_UPDATE_DESC_METHOD(updateBufferDescriptor) {
	const auto &descSets = error::at(_descSets, set, "descriptor sets");
	const auto &descSet = error::at(descSets, index, "descriptor sets allocated");
	const auto &buffer = resource::getBuffer(id);
	const auto bi = vk::DescriptorBufferInfo(buffer.get(), 0, vk::WholeSize);
	const auto ds = vk::WriteDescriptorSet()
		.setDstSet(descSet)
		.setDstBinding(binding)
		.setDstArrayElement(offset)
		.setDescriptorCount(1)
		.setDescriptorType(buffer.isStorage() ? vk::DescriptorType::eStorageBuffer : vk::DescriptorType::eUniformBuffer)
		.setBufferInfo(bi);
	core::device().updateDescriptorSets(1, &ds, 0, nullptr);
}

DECLARE_UPDATE_DESC_METHOD(updateUserImageDescriptor) {
	const auto &descSets = error::at(_descSets, set, "descriptor sets");
	const auto &descSet = error::at(descSets, index, "descriptor sets allocated");
	const auto &image = resource::getUserImage(id);
	const auto ii = vk::DescriptorImageInfo(nullptr, image.get(), vk::ImageLayout::eShaderReadOnlyOptimal);
	const auto ds = vk::WriteDescriptorSet()
		.setDstSet(descSet)
		.setDstBinding(binding)
		.setDstArrayElement(offset)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampledImage)
		.setImageInfo(ii);
	core::device().updateDescriptorSets(1, &ds, 0, nullptr);
}

DECLARE_UPDATE_DESC_METHOD(updateSamplerDescriptor) {
	const auto &descSets = error::at(_descSets, set, "descriptor sets");
	const auto &descSet = error::at(descSets, index, "descriptor sets allocated");
	const auto &sampler = resource::getSampler(id);
	const auto ii = vk::DescriptorImageInfo(sampler, nullptr, vk::ImageLayout::eShaderReadOnlyOptimal);
	const auto ds = vk::WriteDescriptorSet()
		.setDstSet(descSet)
		.setDstBinding(binding)
		.setDstArrayElement(offset)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampler)
		.setImageInfo(ii);
	core::device().updateDescriptorSets(1, &ds, 0, nullptr);
}

#undef DECLARE_UPDATE_DESC_METHOD

vk::Viewport adjustViewport(uint32_t ow, uint32_t oh, const vk::Extent2D &extent) {
	float o = static_cast<float>(ow)           / static_cast<float>(oh);
	float n = static_cast<float>(extent.width) / static_cast<float>(extent.height);
	if (n > o) {
		const auto h = static_cast<float>(extent.height);
		const auto w = h * o;
		const auto x = (extent.width - w) / 2.0f;
		return vk::Viewport(x, 0.0f, w, h, 0.0f, 1.0f);
	} else {
		const auto w = static_cast<float>(extent.width);
		const auto h = w / o;
		const auto y = (extent.height - h) / 2.0f;
		return vk::Viewport(0.0f, y, w, h, 0.0f, 1.0f);
	}
}

std::unordered_map<std::string, Pipeline> createPipelines(
	const vk::RenderPass &renderPass,
	const std::vector<PipelineIdAndSubpassIndex> &pss
) {
	if (config::config().pipelines.empty()) {
		return {};
	}
	if (config::config().pipelines.size() != pss.size()) {
		throw "unexpected error: pipeline count is inconsistent.";
	}
	const auto &extent = window::swapchain().getExtent();

	// ビューポート
	std::vector<vk::Viewport> viewports;
	viewports.emplace_back(adjustViewport(config::config().width, config::config().height, extent));
	std::vector<vk::Rect2D> scissors;
	scissors.emplace_back(vk::Offset2D(0, 0), extent);
	const auto vsci = vk::PipelineViewportStateCreateInfo()
		.setViewports(viewports)
		.setScissors(scissors);

	// マルチサンプル
	const auto msci = vk::PipelineMultisampleStateCreateInfo()
		.setRasterizationSamples(vk::SampleCountFlagBits::e1);

	// 作成情報作成
	std::vector<PipelineCreateTemporaryInfos> ctis;
	std::vector<vk::GraphicsPipelineCreateInfo> cis;
	ctis.reserve(config::config().pipelines.size());
	cis.reserve(config::config().pipelines.size());
	for (const auto &ps: pss) {
		ctis.emplace_back(ps.pipelineId, ps.subpassIndex);
		auto &cti = ctis.back();
		cis.push_back(
			vk::GraphicsPipelineCreateInfo()
				.setStages(cti.sscis)
				.setPVertexInputState(&cti.visci)
				.setPInputAssemblyState(&cti.iasci)
				.setPViewportState(&vsci)
				.setPRasterizationState(&cti.rsci)
				.setPMultisampleState(&msci)
				.setPDepthStencilState(&cti.dssci)
				.setPColorBlendState(&cti.cbsci)
				.setLayout(cti.pipelineLayout)
				.setRenderPass(renderPass)
				.setSubpass(cti.subpass)
		);
	}

	// パイプライン作成
	const auto ps = core::device().createGraphicsPipelines(nullptr, cis).value;

	// 作成
	std::unordered_map<std::string, Pipeline> pipelines;
	pipelines.reserve(ps.size());
	for (size_t i = 0; i < ps.size(); ++i) {
		pipelines.try_emplace(
			ctis[i].id,
			ctis[i].id,
			ps[i],
			ctis[i].pipelineLayout,
			std::move(ctis[i].descSetLayouts),
			std::move(ctis[i].descSets)
		);
	}

	return pipelines;
}

} // namespace graphics::renderpass
