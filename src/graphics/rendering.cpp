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
// コマンドバッファ実行の完了を知るためのセマフォ
// プレゼンテーション開始を待機させるために使う
vk::Semaphore g_semaphoreForRenderFinished;

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

Error createSemaphore(const vk::Device &device) {
	try {
		g_semaphoreForRenderFinished = device.createSemaphore({});
	} catch (...) {
		return Error::CreateSemaphoresForSwapchain;
	}
	return Error::None;
}

Error initialize(const Config &config, const vk::Device &device, const vk::CommandPool &commandPool) {
	getClearValues(config);
	CHECK(createRenderPass(config, device));
	CHECK(swapchain::createFramebuffers(device, g_renderPass, g_framebuffers));
	CHECK(createCommandBuffer(device, commandPool));
	CHECK(createSemaphore(device));
	return Error::None;
}

Error render(const vk::Device &device, const vk::Queue &queue) {
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
	CHECK(swapchain::acquireNextImageIndex(device, index));

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
	const auto si = vk::SubmitInfo()
		.setCommandBuffers({g_commandBuffer})
		.setSignalSemaphores({g_semaphoreForRenderFinished});
	try {
		queue.submit(si, nullptr);
	} catch (...) {
		return Error::SubmitRenderCommandBuffer;
	}

	// プレゼンテーション
	CHECK(swapchain::presentation(queue, g_semaphoreForRenderFinished, index));

	// 終了
	return Error::None;
}

void terminate(const vk::Device &device) {
	if (g_semaphoreForRenderFinished) {
		device.destroySemaphore(g_semaphoreForRenderFinished);
		g_semaphoreForRenderFinished = nullptr;
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
