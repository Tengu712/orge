#pragma once

#include <iostream>

#include "../renderpass/renderpass.hpp"
#include "../resource/charatlus.hpp"
#include "../resource/mesh.hpp"
#include "../text/text.hpp"

#include <vulkan/vulkan.hpp>

namespace graphics::renderer {

class RenderContext {
private:
	const uint32_t _index;
	uint32_t _subpassIndex;
	const vk::CommandBuffer &_commandBuffer;
	const resource::Mesh *_mesh;
	const renderpass::RenderPass *_renderPass;
	const renderpass::GraphicsPipeline *_pipeline;

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
		_index(index),
		_subpassIndex(0),
		_commandBuffer(commandBuffer),
		_mesh(nullptr),
		_renderPass(nullptr),
		_pipeline(nullptr)
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
			_subpassIndex = 0;
		}
	}

	void endRenderPass() noexcept {
		if (_renderPass) {
			_commandBuffer.endRenderPass();
			_renderPass = nullptr;
			_pipeline = nullptr;
		}
	}

	void nextSubpass() {
		_currentRenderPass();
		_commandBuffer.nextSubpass(vk::SubpassContents::eInline);
		_subpassIndex += 1;
	}

	void bindPipeline(const std::string &pipelineId, uint32_t const *indices) {
		// NOTE: _currentRenderPass()でレンダーパスの開始を検証できる。
		if (!_pipeline || _pipeline->id() != pipelineId) {
			const auto &pipeline = _currentRenderPass().getPipeline(pipelineId);
			pipeline.bind(_commandBuffer);
			if (indices) {
				pipeline.bindDescriptorSets(_commandBuffer, indices);
			}
			_pipeline = &pipeline;
		}
	}

	void draw(uint32_t instanceCount, uint32_t instanceOffset) const {
		// NOTE: パイプラインがバインドされていないならレンダーパスも始まっていない。
		if (!_pipeline) {
			throw "no pipeline bound.";
		}
		_commandBuffer.drawIndexed(_currentMesh().indexCount(), instanceCount, 0, 0, instanceOffset);
	}

	void drawDirectly(uint32_t vertexCount, uint32_t instanceCount, uint32_t instanceOffset) const {
		// NOTE: パイプラインがバインドされていないならレンダーパスも始まっていない。
		if (!_pipeline) {
			throw "no pipeline bound.";
		}
		_commandBuffer.draw(vertexCount, instanceCount, 0, instanceOffset);
	}

	void drawTexts() {
		const auto &pipeline = _currentRenderPass().getTextRenderingPipeline(_subpassIndex);
		pipeline.updateBufferDescriptor("@buffer-tr@", 0, 0, 0, 0);
		pipeline.updateCharatlusDescriptors();
		pipeline.updateSamplerDescriptor("@sampler-tr@", 1, 0, 1, 0);
		pipeline.bind(_commandBuffer);
		const std::array<uint32_t, 2> indices{0, 0};
		pipeline.bindDescriptorSets(_commandBuffer, indices.data());
		_pipeline = &pipeline;

		const auto &drawCalls = text::getIndices(_currentRenderPass().id(), _subpassIndex);
		if (!drawCalls.empty()) {
			for (const auto &[start, end]: drawCalls) {
				_commandBuffer.draw(4, static_cast<uint32_t>(end - start), 0, start);
			}
		}
	}
};

} // namespace graphics::renderer
