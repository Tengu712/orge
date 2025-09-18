#include "renderer.hpp"

#include "../../error/error.hpp"
#include "../core/core.hpp"
#include "../text/text.hpp"
#include "../window/swapchain.hpp"

namespace graphics::renderer {

vk::UniqueCommandBuffer createCommandBuffer() {
	const auto ai = vk::CommandBufferAllocateInfo()
		.setCommandPool(core::commandPool())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1);
	auto commandBuffers = core::device().allocateCommandBuffersUnique(ai);
	if (commandBuffers.empty()) {
		throw "failed to allocate a command buffer.";
	}
	return std::move(commandBuffers[0]);
}

std::vector<vk::UniqueSemaphore> createSemaphores() {
	const auto count = window::swapchain().getImages().size();
	std::vector<vk::UniqueSemaphore> semaphores;
	semaphores.reserve(count);
	for (size_t i = 0; i < count; ++i) {
		semaphores.push_back(core::device().createSemaphoreUnique({}));
	}
	return semaphores;
}

Renderer::Renderer():
	_commandBuffer(createCommandBuffer()),
	_semaphoreForImageEnabled(core::device().createSemaphoreUnique({})),
	_semaphoreForRenderFinisheds(createSemaphores()),
	_frameInFlightFence(core::device().createFenceUnique({vk::FenceCreateFlagBits::eSignaled}))
{}

void Renderer::begin() {
	_context.reset();

	const auto index = window::swapchain().acquireNextImageIndex(_semaphoreForImageEnabled.get());

	_commandBuffer->reset();
	const auto cbi = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	_commandBuffer->begin(cbi);

	_context.emplace(index, _commandBuffer.get());
}

void Renderer::end() {
	if (!_context) {
		throw "rendering not begun.";
	}
	// TODO: レンダーパス終了忘れも検知したい。

	_commandBuffer->end();

	core::device().resetFences({_frameInFlightFence.get()});

	const vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	const auto &semaphore = error::at(
		_semaphoreForRenderFinisheds,
		_context->currentIndex(),
		"semaphores for waiting for rendering finished"
	);
	const auto si = vk::SubmitInfo()
		.setWaitSemaphores({_semaphoreForImageEnabled.get()})
		.setWaitDstStageMask({waitStage})
		.setCommandBuffers({_commandBuffer.get()})
		.setSignalSemaphores({semaphore.get()});
	core::queue().submit(si, _frameInFlightFence.get());

	window::swapchain().present(semaphore.get(), _context->currentIndex());

	// 前のフレームのGPU処理が完全に終了するまで待機
	if (core::device().waitForFences({_frameInFlightFence.get()}, VK_TRUE, UINT64_MAX) != vk::Result::eSuccess) {
		throw "failed to wait for rendering comletion.";
	}

	_context.reset();
	text::clearLayoutContext();
}

void Renderer::reset() {
	_commandBuffer->reset();
	_context.reset();
	text::clearLayoutContext();
	for (auto &n: _semaphoreForRenderFinisheds) {
		n = core::device().createSemaphoreUnique({});
	}
}

void Renderer::recreateSemaphoreForImageEnabled() {
	_semaphoreForImageEnabled = core::device().createSemaphoreUnique({});
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
