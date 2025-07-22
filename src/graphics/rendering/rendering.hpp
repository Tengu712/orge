#pragma once

#include "../../config/config.hpp"
#include "../mesh/mesh.hpp"
#include "framebuffer/framebuffer.hpp"
#include "pipeline/pipeline.hpp"
#include "swapchain/swapchain.hpp"

#include <vulkan/vulkan.hpp>

namespace graphics::rendering {

class Renderer {
private:
	struct FrameInfo {
		const uint32_t index;
		std::string meshId;
		uint32_t meshIndexCount;
		std::string pipelineId;
	};

	std::unique_ptr<swapchain::Swapchain> _swapchain;
	const vk::RenderPass _renderPass;
	/// 描画処理コマンド用のコマンドバッファ
	/// orgeは描画完了まで待機するので1個で十分
	const vk::CommandBuffer _commandBuffer;
	/// スワップチェインイメージ取得の完了を知るためのセマフォ
	/// コマンドバッファ提出を待機させるために使う
	const vk::Semaphore _semaphoreForImageEnabled;
	/// コマンドバッファ実行の完了を知るためのセマフォ
	/// プレゼンテーション開始を待機させるために使う
	const std::vector<vk::Semaphore> _semaphoreForRenderFinisheds;
	/// フレーム完了を監視するフェンス
	/// 次フレーム開始前にGPU処理完了を待機するために使う
	const vk::Fence _frameInFlightFence;
	std::vector<framebuffer::Framebuffer> _framebuffers;
	std::optional<FrameInfo> _frameInfo;

	void _ensureWhileRendering(const char *emsg) const {
		if (!_frameInfo) {
			throw emsg;
		}
	}

public:
	Renderer(const Renderer &)  = delete;
	Renderer(const Renderer &&) = delete;
	Renderer &operator=(const Renderer &)  = delete;
	Renderer &operator=(const Renderer &&) = delete;

	Renderer() = delete;
	Renderer(
		const config::Config &config,
		const vk::Instance &instance,
		const vk::PhysicalDevice &physicalDevice,
		const vk::Device &device,
		const vk::CommandPool &commandPool
	);

	void destroy(const vk::Instance &instance, const vk::Device &device) const {
		// TODO:
		pipeline::terminate(device);
		for (const auto &n: _framebuffers) {
			n.destroy(device);
		}
		device.destroyFence(_frameInFlightFence);
		for (const auto &n: _semaphoreForRenderFinisheds) {
			device.destroySemaphore(n);
		}
		device.destroySemaphore(_semaphoreForImageEnabled);
		device.destroyRenderPass(_renderPass);
		_swapchain->destroy(instance, device);
	}

	void beginRender(const vk::Device &device);

	void endRender(const vk::Queue &queue);

	void bindMesh(const char *meshId) {
		_ensureWhileRendering("try to bind a mesh before starting rendering.");
		if (meshId != nullptr && meshId != _frameInfo->pipelineId) {
			_frameInfo->meshIndexCount = mesh::bind(_commandBuffer, meshId);
			_frameInfo->meshId = meshId;
		}
	}

	void bindPipeline(const vk::Device &device, const char *pipelineId) {
		_ensureWhileRendering("try to bind a pipeline before starting rendering.");
		if (pipelineId != nullptr && pipelineId != _frameInfo->pipelineId) {
			pipeline::bind(device, _commandBuffer, _framebuffers.at(_frameInfo->index), pipelineId);
			_frameInfo->pipelineId = pipelineId;
		}
	}

	void bindDescriptorSets(const char *id, uint32_t const *indices) const {
		_ensureWhileRendering("try to bind descriptor sets before starting rendering.");
		pipeline::bindDescriptorSets(_commandBuffer, id, indices);
	}

	void nextSubpass() const {
		_ensureWhileRendering("try to go to next subpass before starting rendering.");
		_commandBuffer.nextSubpass(vk::SubpassContents::eInline);
	}

	void draw(uint32_t instanceCount, uint32_t instanceOffset) const {
		_ensureWhileRendering("try to draw before starting rendering.");
		_commandBuffer.drawIndexed(_frameInfo->meshIndexCount, instanceCount, 0, 0, instanceOffset);
	}

	void toggleFullscreen(
		const config::Config &config,
		const vk::Instance &instance,
		const vk::PhysicalDevice &physicalDevice,
		const vk::Device &device
	);
};

} // namespace graphics::rendering
