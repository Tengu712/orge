#pragma once

#include "../../error/error.hpp"
#include "pipeline.hpp"

#include <unordered_map>
#include <vulkan/vulkan.hpp>

namespace graphics::renderpass {

class RenderPass {
private:
	const std::string _id;
	const vk::UniqueRenderPass _renderPass;
	const std::vector<vk::ClearValue> _clearValues;
	std::vector<vk::UniqueFramebuffer> _framebuffers;
	std::unordered_map<std::string, GraphicsPipeline> _pipelines;
	std::unordered_map<uint32_t, GraphicsPipeline> _trPipelines;

public:
	RenderPass() = delete;
	RenderPass(const RenderPass &) = delete;
	RenderPass &operator =(const RenderPass &) = delete;

	RenderPass(const std::string &id);

	const std::string &id() const noexcept {
		return _id;
	}

	// NOTE: プロキシメソッドがしんどいので、
	//       デメテルの法則ガン無視でPipelineインスタンスにアクセスさせる。
	const GraphicsPipeline &getPipeline(const std::string &id) const {
		return error::at(_pipelines, id, "pipelines");
	}

	// NOTE: プロキシメソッドがしんどいので、
	//       デメテルの法則ガン無視でPipelineインスタンスにアクセスさせる。
	const GraphicsPipeline &getTextRenderingPipeline(uint32_t subpassIndex) const {
		return error::at(_trPipelines, subpassIndex, "text rendering pipelines");
	}

	void begin(const vk::CommandBuffer &commandBuffer, uint32_t index) const noexcept;

	void destroyFramebuffersAndPipelines() noexcept;
	void createFramebuffersAndPipelines();
};

void initializeRenderPasses();

void destroyRenderPasses() noexcept;

const RenderPass &getRenderPass(const std::string &id);

void destroyAllFramebuffersAndPipelines() noexcept;

void createAllFramebuffersAndPipelines();

} // namespace graphics::renderpass
