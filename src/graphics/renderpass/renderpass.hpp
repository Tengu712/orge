#pragma once

#include "pipeline.hpp"

#include <vulkan/vulkan.hpp>

namespace graphics::renderpass {

class RenderPass {
private:
	const std::string _id;
	const vk::RenderPass _renderPass;
	const std::vector<vk::ClearValue> _clearValues;
	std::vector<vk::Framebuffer> _framebuffers;
	std::unordered_map<std::string, Pipeline> _pipelines;

public:
	RenderPass() = delete;
	RenderPass(const RenderPass &) = delete;
	RenderPass &operator =(const RenderPass &) = delete;

	RenderPass(const std::string &id);
	~RenderPass();

	const std::string &id() const noexcept {
		return _id;
	}

	// NOTE: プロキシメソッドがしんどいので、
	//       デメテルの法則ガン無視でPipelineインスタンスにアクセスさせる。
	const Pipeline &getPipeline(const std::string &id) const {
		return error::at(_pipelines, id, "pipelines");
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
