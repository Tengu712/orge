#pragma once

#include "../renderpass/renderpass.hpp"
#include "../resource/mesh.hpp"

#include <vulkan/vulkan.hpp>

namespace graphics::renderer {

class RenderContext {
private:
	const uint32_t _index;
	const vk::CommandBuffer &_commandBuffer;
	const resource::Mesh *_mesh;
	const renderpass::RenderPass *_renderPass;
	const renderpass::Pipeline *_pipeline;

	const resource::Mesh &_currentMesh() const {
		if (_mesh) {
			return *_mesh;
		} else {
			throw "no mesh bound.";
		}
	}

	const renderpass::RenderPass &_currentRenderPass() const {
		if (_renderPass) {
			return *_renderPass;
		} else {
			throw "no render pass bound.";
		}
	}

public:
	RenderContext(uint32_t index, const vk::CommandBuffer &commandBuffer):
		_index(index), _commandBuffer(commandBuffer), _mesh(nullptr), _renderPass(nullptr), _pipeline(nullptr)
	{}

	uint32_t currentIndex() const noexcept {
		return _index;
	}

	void bindMesh(const std::string &meshId) {
		if (!_mesh || _mesh->id() != meshId) {
			auto &mesh = resource::getMesh(meshId);
			mesh.bind(_commandBuffer);
			_mesh = &mesh;
		}
	}

	void beginRenderPass(const std::string &renderPassId) {
		if (_renderPass) {
			throw std::format("render pass '{}' not ended.", _renderPass->id());
		} else {
			auto &renderPass = renderpass::getRenderPass(renderPassId);
			renderPass.begin(_commandBuffer, _index);
			_renderPass = &renderPass;
		}
	}

	void endRenderPass() noexcept {
		if (_renderPass) {
			_commandBuffer.endRenderPass();
			_renderPass = nullptr;
			_pipeline = nullptr;
		}
	}

	void nextSubpass() const {
		_currentRenderPass();
		_commandBuffer.nextSubpass(vk::SubpassContents::eInline);
	}

	void bindPipeline(const std::string &pipelineId, uint32_t const *indices) {
		// NOTE: _currentRenderPass()でレンダーパスの開始を検証できる。
		if (!_pipeline || _pipeline->id() != pipelineId) {
			const auto &pipeline = _currentRenderPass().getPipeline(pipelineId);
			pipeline.bind(_commandBuffer);
			pipeline.bindDescriptorSets(_commandBuffer, indices);
			_pipeline = &pipeline;
		}
	}

	void draw(uint32_t indexCount, uint32_t instanceCount, uint32_t instanceOffset) const {
		// NOTE: パイプラインがバインドされていないならレンダーパスも始まっていない。
		if (!_pipeline) {
			throw "no pipeline bound.";
		}
		const auto ic = indexCount == 0 ? _currentMesh().indexCount() : indexCount;
		_commandBuffer.drawIndexed(ic, instanceCount, 0, 0, instanceOffset);
	}
};

} // namespace graphics::renderer
