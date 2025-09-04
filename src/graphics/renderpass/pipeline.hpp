#pragma once

#include "../../error/error.hpp"

#include <vulkan/vulkan.hpp>

namespace graphics::renderpass {

class Pipeline {
private:
	const std::string _id;
	const vk::Pipeline _pipeline;
	const vk::PipelineLayout _pipelineLayout;
	const std::vector<vk::DescriptorSetLayout> _descSetLayouts;
	const std::vector<std::vector<vk::DescriptorSet>> _descSets;

public:
	Pipeline() = delete;
	Pipeline(const Pipeline &) = delete;
	Pipeline &operator =(const Pipeline &) = delete;

	Pipeline(
		const std::string id,
		const vk::Pipeline &pipeline,
		const vk::PipelineLayout &pipelineLayout,
		const std::vector<vk::DescriptorSetLayout> &&descSetLayouts,
		const std::vector<std::vector<vk::DescriptorSet>> &&descSets
	):
		_id(id),
		_pipeline(pipeline),
		_pipelineLayout(pipelineLayout),
		_descSetLayouts(descSetLayouts),
		_descSets(descSets)
	{}
	~Pipeline();

	const std::string &id() const noexcept {
		return _id;
	}

	void bind(const vk::CommandBuffer &commandBuffer) const noexcept {
		// TODO: コンピュートシェーダに対応する。
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);
	}

	void bindDescriptorSets(const vk::CommandBuffer &commandBuffer, uint32_t const *indices) const {
		std::vector<vk::DescriptorSet> sets;
		sets.reserve(_descSets.size());
		for (size_t i = 0; i < _descSets.size(); ++i) {
			sets.push_back(error::at(_descSets[i], indices[i], "descriptor sets allocated"));
		}
		// TODO: コンピュートシェーダに対応する。
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

	void updateInputAttachmentDescriptor(
		const std::string &id,
		uint32_t set,
		uint32_t index,
		uint32_t binding,
		uint32_t offset,
		uint32_t frameIndex
	) const;

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
};

struct PipelineIdAndSubpassIndex {
	const std::string pipelineId;
	const uint32_t subpassIndex;

	PipelineIdAndSubpassIndex(const std::string &pipelineId, uint32_t subpassIndex):
		pipelineId(pipelineId),
		subpassIndex(subpassIndex)
	{}
};

std::unordered_map<std::string, Pipeline> createPipelines(
	const vk::RenderPass &renderPass,
	const std::string &renderPassId
);

} // namespace graphics::renderpass
