#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics::compute {

class ComputePipeline {
private:
	const std::string _id;
	const std::vector<vk::UniqueDescriptorSetLayout> _descSetLayouts;
	const std::vector<std::vector<vk::UniqueDescriptorSet>> _descSetss;
	const vk::UniquePipelineLayout _pipelineLayout;
	const vk::UniquePipeline _pipeline;

public:
	ComputePipeline() = delete;
	ComputePipeline(const ComputePipeline &) = delete;
	ComputePipeline &operator =(const ComputePipeline &) = delete;

	ComputePipeline(
		const std::string &id,
		std::vector<vk::UniqueDescriptorSetLayout> &&descSetLayouts,
		std::vector<std::vector<vk::UniqueDescriptorSet>> &&descSetss,
		vk::UniquePipelineLayout &&pipelineLayout,
		vk::UniquePipeline &&pipeline
	):
		_id(id),
		_descSetLayouts(std::move(descSetLayouts)),
		_descSetss(std::move(descSetss)),
		_pipelineLayout(std::move(pipelineLayout)),
		_pipeline(std::move(pipeline))
	{}

	const std::string &id() const noexcept {
		return _id;
	}

	void bind(const vk::CommandBuffer &commandBuffer) const noexcept {
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, _pipeline.get());
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
};

void destroyAllComputePipelines();

void initializeComputePipelines();

ComputePipeline &getComputePipeline(const std::string &id);

} // namespace graphics::compute
