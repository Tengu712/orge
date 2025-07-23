#pragma once

#include "../config/config.hpp"
#include "descpool.hpp"
#include "mesh.hpp"
#include "framebuffer.hpp"
#include "pipeline.hpp"
#include "swapchain.hpp"

namespace graphics {

class Renderer {
private:
	struct FrameInfo {
		const uint32_t index;
		std::string meshId;
		uint32_t meshIndexCount;
		std::string pipelineId;
	};

	Swapchain _swapchain;
	const vk::RenderPass _renderPass;
	/// 描画処理コマンド用のコマンドバッファ
	/// orgeは描画完了まで待機するので1個で十分
	const vk::CommandBuffer _commandBuffer;
	/// スワップチェインイメージ取得の完了を知るためのセマフォ
	/// コマンドバッファ提出を待機させるために使う
	vk::Semaphore _semaphoreForImageEnabled;
	/// コマンドバッファ実行の完了を知るためのセマフォ
	/// プレゼンテーション開始を待機させるために使う
	const std::vector<vk::Semaphore> _semaphoreForRenderFinisheds;
	/// フレーム完了を監視するフェンス
	/// 次フレーム開始前にGPU処理完了を待機するために使う
	const vk::Fence _frameInFlightFence;
	std::vector<Framebuffer> _framebuffers;
	vk::DescriptorPool _descPool;
	std::unordered_map<std::string, Pipeline> _pipelines;
	std::optional<FrameInfo> _frameInfo;

	void _ensureWhileRendering(const char *emsg) const {
		if (!_frameInfo) {
			throw emsg;
		}
	}

	void _destroyForRecreatingSwapchainOrSurface(const vk::Device &device) {
		for (const auto &n: _pipelines) {
			n.second.destroy(device);
		}
		device.destroyDescriptorPool(_descPool);
		for (const auto &n: _framebuffers) {
			n.destroy(device);
		}
		_framebuffers.clear();
		// NOTE: vk::Result::eSuboptimalKHRはセマフォをシグナルするらしいので、
		//       直後のacuireNextImageKHRでヴァリデーションエラーが出ないように再作成。
		device.destroy(_semaphoreForImageEnabled);
		_semaphoreForImageEnabled = device.createSemaphore({});
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
		for (const auto &n: _pipelines) {
			n.second.destroy(device);
		}
		if (_descPool) {
			device.destroyDescriptorPool(_descPool);
		}
		for (const auto &n: _framebuffers) {
			n.destroy(device);
		}
		device.destroyFence(_frameInFlightFence);
		for (const auto &n: _semaphoreForRenderFinisheds) {
			device.destroySemaphore(n);
		}
		device.destroySemaphore(_semaphoreForImageEnabled);
		device.destroyRenderPass(_renderPass);
		_swapchain.destroy(instance, device);
	}

	const Pipeline &getPipeline(const char *id) const {
		return _pipelines.at(id);
	}

	void beginRender(const vk::Device &device);

	void endRender(const vk::Device &device, const vk::Queue &queue);

	void bindMesh(const char *meshId, const Mesh &mesh) {
		_ensureWhileRendering("try to bind a mesh before starting rendering.");
		if (meshId != nullptr && meshId != _frameInfo->pipelineId) {
			mesh.bind(_commandBuffer);
			_frameInfo->meshIndexCount = mesh.getIndexCount();
			_frameInfo->meshId = meshId;
		}
	}

	void bindPipeline(const vk::Device &device, const char *pipelineId) {
		_ensureWhileRendering("try to bind a pipeline before starting rendering.");
		if (pipelineId != nullptr && pipelineId != _frameInfo->pipelineId) {
			_pipelines.at(pipelineId).bind(_commandBuffer);
			_pipelines.at(pipelineId).updateInputAttachmentDescriptors(device, _framebuffers.at(_frameInfo->index));
			_frameInfo->pipelineId = pipelineId;
		}
	}

	void bindDescriptorSets(const char *pipelineId, uint32_t const *indices) const {
		_ensureWhileRendering("try to bind descriptor sets before starting rendering.");
		_pipelines.at(pipelineId).bindDescriptorSets(_commandBuffer, indices);
	}

	void nextSubpass() const {
		_ensureWhileRendering("try to go to next subpass before starting rendering.");
		_commandBuffer.nextSubpass(vk::SubpassContents::eInline);
	}

	void draw(uint32_t instanceCount, uint32_t instanceOffset) const {
		_ensureWhileRendering("try to draw before starting rendering.");
		_commandBuffer.drawIndexed(_frameInfo->meshIndexCount, instanceCount, 0, 0, instanceOffset);
	}

	void recreateSwapchain(
		const config::Config &config,
		const vk::PhysicalDevice &physicalDevice,
		const vk::Device &device
	);

	void recreateSurface(
		const config::Config &config,
		const vk::Instance &instance,
		const vk::PhysicalDevice &physicalDevice,
		const vk::Device &device
	);

	void toggleFullscreen() const noexcept {
		_swapchain.toggleFullscreen();
	}
};

} // namespace graphics
