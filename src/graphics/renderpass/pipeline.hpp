#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics::renderpass {

class GraphicsPipeline {
private:
	const std::string _id;
	const vk::Pipeline _pipeline;
	const vk::PipelineLayout _pipelineLayout;
	const std::vector<vk::DescriptorSetLayout> _descSetLayouts;
	const std::vector<std::vector<vk::DescriptorSet>> _descSets;

public:
	GraphicsPipeline() = delete;
	GraphicsPipeline(const GraphicsPipeline &) = delete;
	GraphicsPipeline &operator =(const GraphicsPipeline &) = delete;

	GraphicsPipeline(
		const std::string &id,
		const vk::Pipeline &pipeline,
		const vk::PipelineLayout &pipelineLayout,
		const std::vector<vk::DescriptorSetLayout> &&descSetLayouts,
		const std::vector<std::vector<vk::DescriptorSet>> &&descSets
	):
		_id(id),
		_pipeline(pipeline),
		_pipelineLayout(pipelineLayout),
		_descSetLayouts(std::move(descSetLayouts)),
		_descSets(std::move(descSets))
	{}
	~GraphicsPipeline();

	const std::string &id() const noexcept {
		return _id;
	}

	void bind(const vk::CommandBuffer &commandBuffer) const noexcept {
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);
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
};

std::unordered_map<std::string, GraphicsPipeline> createPipelines(
	const vk::RenderPass &renderPass,
	const std::string &renderPassId
);

} // namespace graphics::renderpass
