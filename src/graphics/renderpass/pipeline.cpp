#include "pipeline.hpp"

#include "../../config/config.hpp"
#include "../../error/error.hpp"
#include "../core/core.hpp"
#include "../resource/buffer.hpp"
#include "../resource/charatlus.hpp"
#include "../resource/image-attachment.hpp"
#include "../resource/image-user.hpp"
#include "../resource/sampler.hpp"
#include "pipeline-text.hpp"
#include "pipeline-user.hpp"

namespace graphics::renderpass {

void GraphicsPipeline::bindDescriptorSets(const vk::CommandBuffer &commandBuffer, uint32_t const *indices) const {
	std::vector<vk::DescriptorSet> sets;
	sets.reserve(_descSets.size());
	for (size_t i = 0; i < _descSets.size(); ++i) {
		sets.push_back(error::at(_descSets[i], indices[i], "descriptor sets allocated").get());
	}
	commandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		_pipelineLayout.get(),
		0,
		static_cast<uint32_t>(sets.size()),
		sets.data(),
		0,
		nullptr
	);
}

#define DECLARE_UPDATE_DESC_METHOD(n) \
	void GraphicsPipeline::n( \
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
		.setDstSet(descSet.get())
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
		.setDstSet(descSet.get())
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
		.setDstSet(descSet.get())
		.setDstBinding(binding)
		.setDstArrayElement(offset)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampler)
		.setImageInfo(ii);
	core::device().updateDescriptorSets(1, &ds, 0, nullptr);
}

#undef DECLARE_UPDATE_DESC_METHOD

void GraphicsPipeline::updateInputAttachmentDescriptor(
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
		.setDstSet(descSet.get())
		.setDstBinding(binding)
		.setDstArrayElement(offset)
		.setDescriptorCount(1)
		.setDescriptorType(isInputAttachment ? vk::DescriptorType::eInputAttachment : vk::DescriptorType::eSampledImage)
		.setImageInfo(ii);
	core::device().updateDescriptorSets(1, &ds, 0, nullptr);
}

void GraphicsPipeline::updateCharatlusDescriptors() const {
	const auto &descSets = error::at(_descSets, 1, "descriptor sets");
	const auto &descSet = error::at(descSets, 0, "descriptor sets allocated");
	std::vector<vk::WriteDescriptorSet> sets;
	sets.reserve(resource::charAtluses().size());
	for (const auto &[id, n]: resource::charAtluses()) {
		const auto offset = config::config().fontMap.at(id);
		const auto ii = vk::DescriptorImageInfo(nullptr, n.get(), vk::ImageLayout::eShaderReadOnlyOptimal);
		sets.push_back(
			vk::WriteDescriptorSet()
				.setDstSet(descSet.get())
				.setDstBinding(0)
				.setDstArrayElement(offset)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eSampledImage)
				.setImageInfo(ii)
		);
	}
	core::device().updateDescriptorSets(static_cast<uint32_t>(sets.size()), sets.data(), 0, nullptr);
}

std::unordered_map<std::string, GraphicsPipeline> createPipelines(
	const vk::RenderPass &renderPass,
	const std::string &renderPassId
) {
	if (config::config().pipelines.empty()) {
		return {};
	}

	const auto &rpconfig = config::config().renderPasses.at(renderPassId);

	std::unordered_map<std::string, GraphicsPipeline> pipelines;
	for (size_t i = 0; i < rpconfig.subpasses.size(); ++i) {
		for (const auto &n: rpconfig.subpasses[i].pipelines) {
			if (n != "@text@") {
				createGraphicsPipeline(renderPass, n, static_cast<uint32_t>(i), pipelines);
			}
		}
	}

	// TODO: これパフォーマンスは大丈夫？
	return pipelines;
}

std::unordered_map<uint32_t, GraphicsPipeline> createTextRenderingPipelines(
	const vk::RenderPass &renderPass,
	const std::string &renderPassId
) {
	const auto &rpconfig = config::config().renderPasses.at(renderPassId);
	std::unordered_map<uint32_t, GraphicsPipeline> pipelines;
	for (size_t i = 0; i < rpconfig.subpasses.size(); ++i) {
		for (const auto &n: rpconfig.subpasses[i].pipelines) {
			if (n == "@text@") {
				createTextRenderingPipeline(renderPass, static_cast<uint32_t>(i), pipelines);
			}
		}
	}
	// TODO: これパフォーマンスは大丈夫？
	return pipelines;
}

} // namespace graphics::renderpass
