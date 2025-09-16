#include "pipeline.hpp"

#include "../../error/error.hpp"

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

} // namespace graphics::compute
