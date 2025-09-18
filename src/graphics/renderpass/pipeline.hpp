#pragma once

#include <unordered_map>
#include <vulkan/vulkan.hpp>

namespace graphics::renderpass {

class GraphicsPipeline {
private:
	const std::string _id;
	const vk::UniquePipeline _pipeline;
	const vk::UniquePipelineLayout _pipelineLayout;
	const std::vector<vk::UniqueDescriptorSetLayout> _descSetLayouts;
	const std::vector<std::vector<vk::UniqueDescriptorSet>> _descSets;

public:
	GraphicsPipeline() = delete;
	GraphicsPipeline(const GraphicsPipeline &) = delete;
	GraphicsPipeline &operator =(const GraphicsPipeline &) = delete;

	GraphicsPipeline(
		const std::string &id,
		vk::UniquePipeline &&pipeline,
		vk::UniquePipelineLayout &&pipelineLayout,
		std::vector<vk::UniqueDescriptorSetLayout> &&descSetLayouts,
		std::vector<std::vector<vk::UniqueDescriptorSet>> &&descSets
	):
		_id(id),
		_pipeline(std::move(pipeline)),
		_pipelineLayout(std::move(pipelineLayout)),
		_descSetLayouts(std::move(descSetLayouts)),
		_descSets(std::move(descSets))
	{}

	const std::string &id() const noexcept {
		return _id;
	}

	void bind(const vk::CommandBuffer &commandBuffer) const noexcept {
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline.get());
	}

	void bindDescriptorSets(const vk::CommandBuffer &commandBuffer, uint32_t const *indices) const;

#define DECLARE_UPDATE_DESC_METHOD(n) \
	void n( \
		const std::string &id, \
		uint32_t set, \
		uint32_t index, \
		uint32_t binding, \
		uint32_t offset \
	) const

	DECLARE_UPDATE_DESC_METHOD(updateBufferDescriptor);
	DECLARE_UPDATE_DESC_METHOD(updateUserImageDescriptor);
	DECLARE_UPDATE_DESC_METHOD(updateSamplerDescriptor);

#undef DECLARE_UPDATE_DESC_METHOD

	void updateInputAttachmentDescriptor(
		const std::string &id,
		uint32_t set,
		uint32_t index,
		uint32_t binding,
		uint32_t offset,
		uint32_t frameIndex
	) const;

	void updateCharatlusDescriptors() const;
};

std::unordered_map<std::string, GraphicsPipeline> createPipelines(
	const vk::RenderPass &renderPass,
	const std::string &renderPassId
);

std::unordered_map<uint32_t, GraphicsPipeline> createTextRenderingPipelines(
	const vk::RenderPass &renderPass,
	const std::string &renderPassId
);

} // namespace graphics::renderpass
