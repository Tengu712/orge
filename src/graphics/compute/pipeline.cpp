#include "pipeline.hpp"

#include "../../error/error.hpp"
#include "../core/core.hpp"
#include "../resource/buffer.hpp"
#include "../resource/image-storage.hpp"
#include "../resource/image-user.hpp"
#include "../resource/sampler.hpp"

namespace graphics::compute {

void ComputePipeline::bindDescriptorSets(const vk::CommandBuffer &commandBuffer, uint32_t const *indices) const {
	std::vector<vk::DescriptorSet> sets;
	sets.reserve(_descSetss.size());
	for (size_t i = 0; i < _descSetss.size(); ++i) {
		sets.push_back(error::at(_descSetss[i], indices[i], "descriptor sets allocated").get());
	}
	commandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eCompute,
		_pipelineLayout.get(),
		0,
		static_cast<uint32_t>(sets.size()),
		sets.data(),
		0,
		nullptr
	);
}

#define DEFINE_UPDATE_DESC_METHOD(n) \
	void ComputePipeline::n( \
		const std::string &id, \
		uint32_t set, \
		uint32_t index, \
		uint32_t binding, \
		uint32_t offset \
	) const

DEFINE_UPDATE_DESC_METHOD(updateBufferDescriptor) {
	const auto &descSets = error::at(_descSetss, set, "descriptor sets");
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

DEFINE_UPDATE_DESC_METHOD(updateImageDescriptor) {
	const auto &descSets = error::at(_descSetss, set, "descriptor sets");
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

DEFINE_UPDATE_DESC_METHOD(updateStorageImageDescriptor) {
	const auto &descSets = error::at(_descSetss, set, "descriptor sets");
	const auto &descSet = error::at(descSets, index, "descriptor sets allocated");
	const auto &image = resource::getStorageImage(id);
	const auto ii = vk::DescriptorImageInfo(nullptr, image.get(), vk::ImageLayout::eGeneral);
	const auto ds = vk::WriteDescriptorSet()
		.setDstSet(descSet.get())
		.setDstBinding(binding)
		.setDstArrayElement(offset)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eStorageImage)
		.setImageInfo(ii);
	core::device().updateDescriptorSets(1, &ds, 0, nullptr);
}

DEFINE_UPDATE_DESC_METHOD(updateSamplerDescriptor) {
	const auto &descSets = error::at(_descSetss, set, "descriptor sets");
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

} // namespace graphics::compute
