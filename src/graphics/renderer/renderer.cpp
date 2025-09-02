#include "renderer.hpp"

#include "../../error/error.hpp"
#include "../core/core.hpp"
#include "../window/swapchain.hpp"

namespace graphics::renderer {

vk::CommandBuffer createCommandBuffer() {
	const auto ai = vk::CommandBufferAllocateInfo()
		.setCommandPool(core::commandPool())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1);
	return error::at(core::device().allocateCommandBuffers(ai), 0, "command buffers allocated");
}

std::vector<vk::Semaphore> createSemaphores() {
	const auto count = window::swapchain().getImages().size();
	std::vector<vk::Semaphore> semaphores;
	semaphores.reserve(count);
	for (size_t i = 0; i < count; ++i) {
		semaphores.push_back(core::device().createSemaphore({}));
	}
	return semaphores;
}

Renderer::Renderer():
	_commandBuffer(createCommandBuffer()),
	_semaphoreForImageEnabled(core::device().createSemaphore({})),
	_semaphoreForRenderFinisheds(createSemaphores()),
	_frameInFlightFence(core::device().createFence({vk::FenceCreateFlagBits::eSignaled}))
{}

Renderer::~Renderer() {
	core::device().destroy(_frameInFlightFence);
	for (const auto &n: _semaphoreForRenderFinisheds) {
		core::device().destroy(n);
	}
	core::device().destroy(_semaphoreForImageEnabled);
}

void Renderer::begin() {
	_context.reset();

	const auto index = window::swapchain().acquireNextImageIndex(_semaphoreForImageEnabled);

	_commandBuffer.reset();
	const auto cbi = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	_commandBuffer.begin(cbi);

	_context.emplace(index, _commandBuffer);
}

void Renderer::end() {
	if (!_context) {
		throw "rendering not begun.";
	}
	// TODO: レンダーパス終了忘れも検知したい。

	_commandBuffer.end();

	core::device().resetFences({_frameInFlightFence});

	const vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	const auto &semaphore = error::at(
		_semaphoreForRenderFinisheds,
		_context->currentIndex(),
		"semaphores for waiting for rendering finished"
	);
	const auto si = vk::SubmitInfo()
		.setWaitSemaphores({_semaphoreForImageEnabled})
		.setWaitDstStageMask({waitStage})
		.setCommandBuffers({_commandBuffer})
		.setSignalSemaphores({semaphore});
	core::queue().submit(si, _frameInFlightFence);

	window::swapchain().present(semaphore, _context->currentIndex());

	// 前のフレームのGPU処理が完全に終了するまで待機
	if (core::device().waitForFences({_frameInFlightFence}, VK_TRUE, UINT64_MAX) != vk::Result::eSuccess) {
		throw "failed to wait for rendering comletion.";
	}

	_context.reset();
}

void Renderer::reset() {
	_commandBuffer.reset();
	for (auto &n: _semaphoreForRenderFinisheds) {
		core::device().destroySemaphore(n);
		n = core::device().createSemaphore({});
	}
	_context.reset();
}

void Renderer::recreateSemaphoreForImageEnabled() {
	core::device().destroy(_semaphoreForImageEnabled);
	_semaphoreForImageEnabled = core::device().createSemaphore({});
}

std::optional<Renderer> g_renderer;

void initializeRenderer() {
	if (g_renderer) {
		throw "renderer already initialized.";
	}
	g_renderer.emplace();
}

void destroyRenderer() noexcept {
	g_renderer.reset();
}

Renderer &renderer() {
	if (g_renderer) {
		return g_renderer.value();
	} else {
		throw "renderer not initialized.";
	}
}

} // namespace graphics::renderer
