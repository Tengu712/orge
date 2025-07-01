#include "rendering.hpp"

#include "swapchain.hpp"

#include <vector>

namespace graphics::rendering {

std::vector<vk::ClearValue> g_clearValues;
vk::RenderPass g_renderPass;
std::vector<vk::Framebuffer> g_framebuffers;
// 描画処理コマンド用のコマンドバッファ
// orgeはプレゼンテーション時に描画完了まで待機するので1個で十分
vk::CommandBuffer g_commandBuffer;
// スワップチェインイメージ取得の完了を知るためのセマフォ
// コマンドバッファ提出を待機させるために使う
vk::Semaphore g_semaphoreForImageEnabled;
// コマンドバッファ実行の完了を知るためのセマフォ
// プレゼンテーション開始を待機させるために使う
std::vector<vk::Semaphore> g_semaphoreForRenderFinisheds;
// フレーム完了を監視するフェンス
// 次フレーム開始前にGPU処理完了を待機するために使う
vk::Fence g_frameInFlightFence;

void getClearValues(const Config &config) {
	for (const auto &n: config.attachments) {
		g_clearValues.push_back(n.clearValue);
	}
}

Error createRenderPass(const Config &config, const vk::Device &device) {
	std::vector<vk::AttachmentDescription> attachments;
	for (const auto &n: config.attachments) {
		attachments.emplace_back(
			vk::AttachmentDescriptionFlags(),
			n.format,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			n.discard ? vk::AttachmentStoreOp::eNone : vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			n.finalLayout
		);
	}

	std::vector<vk::SubpassDescription> subpasses;
	for (const auto &n: config.subpasses) {
		subpasses.emplace_back(
			vk::SubpassDescription()
				.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
				.setInputAttachments(n.inputs)
				.setColorAttachments(n.outputs)
				.setPDepthStencilAttachment(n.depth ? &(*n.depth) : nullptr)
		);
	}

	std::vector<vk::SubpassDependency> subpassDeps;
	for (const auto &n: config.subpassDeps) {
		subpassDeps.emplace_back(
			n.src,
			n.dst,
			vk::PipelineStageFlagBits::eAllCommands,
			vk::PipelineStageFlagBits::eAllCommands,
			vk::AccessFlagBits::eMemoryWrite,
			vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite
		);
	}

	const auto ci = vk::RenderPassCreateInfo()
		.setAttachments(attachments)
		.setSubpasses(subpasses)
		.setDependencies(subpassDeps);
	try {
		g_renderPass = device.createRenderPass(ci);
	} catch (...) {
		return Error::CreateRenderPass;
	}

	return Error::None;
}

Error createCommandBuffer(const vk::Device &device, const vk::CommandPool &commandPool) {
	const auto ai = vk::CommandBufferAllocateInfo()
		.setCommandPool(commandPool)
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1);
	try {
		g_commandBuffer = device.allocateCommandBuffers(ai).at(0);
	} catch (...) {
		return Error::CreateRenderCommandBuffer;
	}
	return Error::None;
}

Error createSemaphores(const vk::Device &device) {
	try {
		g_semaphoreForImageEnabled = device.createSemaphore({});
		g_semaphoreForRenderFinisheds.reserve(swapchain::getImageCount());
		for (int i = 0; i < swapchain::getImageCount(); ++i) {
			g_semaphoreForRenderFinisheds.push_back(device.createSemaphore({}));
		}
	} catch (...) {
		return Error::CreateSemaphoresForRendering;
	}
	return Error::None;
}

Error createFence(const vk::Device &device) {
	try {
		g_frameInFlightFence = device.createFence({vk::FenceCreateFlagBits::eSignaled});
	} catch (...) {
		return Error::CreateFenceForRendering;
	}
	return Error::None;
}

Error initialize(const Config &config, const vk::Device &device, const vk::CommandPool &commandPool) {
	getClearValues(config);
	CHECK(createRenderPass(config, device));
	CHECK(swapchain::createFramebuffers(device, g_renderPass, g_framebuffers));
	CHECK(createCommandBuffer(device, commandPool));
	CHECK(createSemaphores(device));
	CHECK(createFence(device));
	return Error::None;
}

Error render(const vk::Device &device, const vk::Queue &queue) {
	// 前のフレームのGPU処理が完全に終了するまで待機
	try {
		if (device.waitForFences({g_frameInFlightFence}, VK_TRUE, UINT64_MAX) != vk::Result::eSuccess) {
			return Error::WaitForRenderingFence;
		}
		device.resetFences({g_frameInFlightFence});
	} catch (...) {
		return Error::WaitForRenderingFence;
	}

	// コマンドバッファリセット
	try {
		g_commandBuffer.reset();
	} catch (...) {
		return Error::ResetRenderCommandBuffer;
	}

	// コマンドバッファ開始
	const auto cbi = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	try {
		g_commandBuffer.begin(cbi);
	} catch (...) {
		return Error::BeginRenderCommandBuffer;
	}

	// スワップチェインイメージ番号取得
	uint32_t index;
	CHECK(swapchain::acquireNextImageIndex(device, g_semaphoreForImageEnabled, index));

	// レンダーパス開始
	const auto rbi = vk::RenderPassBeginInfo()
		.setRenderPass(g_renderPass)
		.setFramebuffer(g_framebuffers[index])
		.setRenderArea(vk::Rect2D({0, 0}, swapchain::getImageSize()))
		.setClearValues(g_clearValues);
	g_commandBuffer.beginRenderPass(rbi, vk::SubpassContents::eInline);

	// レンダーパス終了
	g_commandBuffer.endRenderPass();

	// コマンドバッファ終了
	try {
		g_commandBuffer.end();
	} catch (...) {
		return Error::EndRenderCommandBuffer;
	}

	// 提出
	try {
		const vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		const auto si = vk::SubmitInfo()
			.setWaitSemaphores({g_semaphoreForImageEnabled})
			.setWaitDstStageMask({waitStage})
			.setCommandBuffers({g_commandBuffer})
			.setSignalSemaphores({g_semaphoreForRenderFinisheds.at(index)});
		queue.submit(si, g_frameInFlightFence);
	} catch (...) {
		return Error::SubmitRenderCommandBuffer;
	}

	// プレゼンテーション
	//
	// NOTE: ここで画面が切り替わるまで待機される。
	CHECK(swapchain::presentation(queue, g_semaphoreForRenderFinisheds.at(index), index));

	// 終了
	return Error::None;
}

void terminate(const vk::Device &device) {
	if (g_frameInFlightFence) {
		device.destroyFence(g_frameInFlightFence);
		g_frameInFlightFence = nullptr;
	}
	if (!g_semaphoreForRenderFinisheds.empty()) {
		for (const auto &semaphore: g_semaphoreForRenderFinisheds) {
			device.destroySemaphore(semaphore);
		}
		g_semaphoreForRenderFinisheds.clear();
	}
	if (g_semaphoreForImageEnabled) {
		device.destroySemaphore(g_semaphoreForImageEnabled);
		g_semaphoreForImageEnabled = nullptr;
	}
	if (g_commandBuffer) {
		// NOTE: コマンドプール全体を破棄するので個別に解放する必要はない。
		g_commandBuffer = nullptr;
	}
	if (!g_framebuffers.empty()) {
		for (auto &framebuffer: g_framebuffers) {
			device.destroyFramebuffer(framebuffer);
		}
		g_framebuffers.clear();
	}
	if (g_renderPass) {
		device.destroyRenderPass(g_renderPass);
		g_renderPass = nullptr;
	}
	g_clearValues.clear();
}

} // namespace graphics::rendering
