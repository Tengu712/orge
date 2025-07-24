#pragma once

#include "../config/config.hpp"
#include "../error/error.hpp"
#include "buffer.hpp"
#include "framebuffer.hpp"
#include "image.hpp"

namespace graphics {

struct InputAttachment {
	const uint32_t id;
	const uint32_t set;
	const uint32_t binding;
	const bool isTexture;

	InputAttachment() = delete;
	InputAttachment(uint32_t id, uint32_t set, uint32_t binding, bool isTexture):
		id(id), set(set), binding(binding), isTexture(isTexture)
	{}
};

class Pipeline {
private:
	const vk::Pipeline _pipeline;
	const vk::PipelineLayout _pipelineLayout;
	const std::vector<vk::DescriptorSetLayout> _descSetLayouts;
	const std::vector<std::vector<vk::DescriptorSet>> _descSets;
	const std::vector<InputAttachment> _inputs;

public:
	Pipeline() = delete;
	Pipeline(
		const vk::Pipeline pipeline,
		const vk::PipelineLayout pipelineLayout,
		const std::vector<vk::DescriptorSetLayout> &&descSetLayouts,
		const std::vector<std::vector<vk::DescriptorSet>> &&descSets,
		const std::vector<InputAttachment> &&inputs
	) :
		_pipeline(pipeline),
		_pipelineLayout(pipelineLayout),
		_descSetLayouts(descSetLayouts),
		_descSets(descSets),
		_inputs(inputs)
	{}

	void destroy(const vk::Device &device) const {
		device.destroyPipeline(_pipeline);
		device.destroyPipelineLayout(_pipelineLayout);
		for (const auto &n: _descSetLayouts) {
			device.destroyDescriptorSetLayout(n);
		}
	}

	void bind(const vk::CommandBuffer &commandBuffer) const noexcept {
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);
	}

	void bindDescriptorSets(const vk::CommandBuffer &commandBuffer, uint32_t const *indices) const {
		std::vector<vk::DescriptorSet> sets;
		sets.reserve(_descSets.size());
		for (size_t i = 0; i < _descSets.size(); ++i) {
			sets.push_back(error::at(_descSets[i], indices[i], "descriptor sets allocated"));
		}
		commandBuffer.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics,
			_pipelineLayout,
			0,
			static_cast<uint32_t>(sets.size()),
			sets.data(),
			0,
			nullptr
		);
	}

	void updateInputAttachmentDescriptors(const vk::Device &device, const Framebuffer &framebuffer) const {
		for (const auto &n: _inputs) {
			const auto &view = framebuffer.getAttachmentView(n.id);
			const auto ii = vk::DescriptorImageInfo(nullptr, view, vk::ImageLayout::eShaderReadOnlyOptimal);
			const auto &descSets = error::at(_descSets, n.set, "descriptor sets");
			std::vector<vk::WriteDescriptorSet> wdss;
			for (const auto &m: descSets) {
				wdss.push_back(
					vk::WriteDescriptorSet()
						.setDstSet(m)
						.setDstBinding(n.binding)
						.setDescriptorCount(1)
						.setDescriptorType(
							n.isTexture
								? vk::DescriptorType::eSampledImage
								: vk::DescriptorType::eInputAttachment
						)
						.setImageInfo(ii)
				);
			}
			device.updateDescriptorSets(static_cast<uint32_t>(descSets.size()), wdss.data(), 0, nullptr);
		}
	}

	void updateBufferDescriptor(
		const vk::Device &device,
		const Buffer &buffer,
		uint32_t set,
		uint32_t index,
		uint32_t binding,
		uint32_t offset
	) const {
		const auto &descSets = error::at(_descSets, set, "descriptor sets");
		const auto &descSet = error::at(descSets, index, "descriptor sets allocated");
		const auto bi = vk::DescriptorBufferInfo(buffer.get(), 0, vk::WholeSize);
		const auto ds = vk::WriteDescriptorSet()
			.setDstSet(descSet)
			.setDstBinding(binding)
			.setDstArrayElement(offset)
			.setDescriptorCount(1)
			.setDescriptorType(buffer.isStorage() ? vk::DescriptorType::eStorageBuffer : vk::DescriptorType::eUniformBuffer)
			.setBufferInfo(bi);
		device.updateDescriptorSets(1, &ds, 0, nullptr);
	}

	void updateImageDescriptor(
		const vk::Device &device,
		const Image &image,
		uint32_t set,
		uint32_t index,
		uint32_t binding,
		uint32_t offset
	) const {
		const auto &descSets = error::at(_descSets, set, "descriptor sets");
		const auto &descSet = error::at(descSets, index, "descriptor sets allocated");
		const auto ii = vk::DescriptorImageInfo(nullptr, image.get(), vk::ImageLayout::eShaderReadOnlyOptimal);
		const auto ds = vk::WriteDescriptorSet()
			.setDstSet(descSet)
			.setDstBinding(binding)
			.setDstArrayElement(offset)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eSampledImage)
			.setImageInfo(ii);
		device.updateDescriptorSets(1, &ds, 0, nullptr);
	}

	void updateSamplerDescriptor(
		const vk::Device &device,
		const vk::Sampler &sampler,
		uint32_t set,
		uint32_t index,
		uint32_t binding,
		uint32_t offset
	) const {
		const auto &descSets = error::at(_descSets, set, "descriptor sets");
		const auto &descSet = error::at(descSets, index, "descriptor sets allocated");
		const auto ii = vk::DescriptorImageInfo(sampler, nullptr, vk::ImageLayout::eShaderReadOnlyOptimal);
		const auto ds = vk::WriteDescriptorSet()
			.setDstSet(descSet)
			.setDstBinding(binding)
			.setDstArrayElement(offset)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eSampler)
			.setImageInfo(ii);
		device.updateDescriptorSets(1, &ds, 0, nullptr);
	}
};

std::unordered_map<std::string, Pipeline> createPipelines(
	const config::Config &config,
	const vk::Device &device,
	const vk::RenderPass &renderPass,
	const vk::DescriptorPool &descPool,
	const vk::Extent2D &extent
);

} // namespace graphics
