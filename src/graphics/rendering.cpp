#include "rendering.hpp"

#include "mesh.hpp"
#include "pipeline.hpp"
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
// 現在のフレームインデックス
uint32_t g_index;
// 現在のインデックスカウント
uint32_t g_indexCount;

void getClearValues(const Config &config) {
	for (const auto &n: config.attachments) {
		g_clearValues.push_back(n.clearValue);
	}
}

void createRenderPass(const Config &config, const vk::Device &device) {
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
	g_renderPass = device.createRenderPass(ci);
}

void createCommandBuffer(const vk::Device &device, const vk::CommandPool &commandPool) {
	const auto ai = vk::CommandBufferAllocateInfo()
		.setCommandPool(commandPool)
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1);
	g_commandBuffer = device.allocateCommandBuffers(ai).at(0);
}

void createSemaphores(const vk::Device &device) {
	g_semaphoreForImageEnabled = device.createSemaphore({});
	g_semaphoreForRenderFinisheds.reserve(swapchain::getImageCount());
	for (int i = 0; i < swapchain::getImageCount(); ++i) {
		g_semaphoreForRenderFinisheds.push_back(device.createSemaphore({}));
	}
}

void createFence(const vk::Device &device) {
	g_frameInFlightFence = device.createFence({vk::FenceCreateFlagBits::eSignaled});
}

void initialize(const Config &config, const vk::Device &device, const vk::CommandPool &commandPool) {
	getClearValues(config);
	createRenderPass(config, device);
	g_framebuffers = swapchain::createFramebuffers(device, g_renderPass);
	createCommandBuffer(device, commandPool);
	createSemaphores(device);
	createFence(device);
	pipeline::initialize(config, device, g_renderPass);
}

void beginRender(const vk::Device &device) {
	// 前のフレームのGPU処理が完全に終了するまで待機
	if (device.waitForFences({g_frameInFlightFence}, VK_TRUE, UINT64_MAX) != vk::Result::eSuccess) {
		throw "failed to wait for rendering comletion.";
	}
	device.resetFences({g_frameInFlightFence});

	// コマンドバッファリセット
	g_commandBuffer.reset();

	// コマンドバッファ開始
	const auto cbi = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	g_commandBuffer.begin(cbi);

	// スワップチェインイメージ番号取得
	g_index = swapchain::acquireNextImageIndex(device, g_semaphoreForImageEnabled);

	// レンダーパス開始
	const auto rbi = vk::RenderPassBeginInfo()
		.setRenderPass(g_renderPass)
		.setFramebuffer(g_framebuffers[g_index])
		.setRenderArea(vk::Rect2D({0, 0}, swapchain::getImageSize()))
		.setClearValues(g_clearValues);
	g_commandBuffer.beginRenderPass(rbi, vk::SubpassContents::eInline);
}

void bindDescriptorSets(
	const char *id,
	uint32_t count,
	uint32_t const *indices
) {
	pipeline::bindDescriptorSets(g_commandBuffer, id, count, indices);
}

void draw(
	uint32_t pipelineCount,
	const char *const *pipelines,
	const char *mesh,
	uint32_t instanceCount,
	uint32_t instanceOffset
) {
	// パイプラインバインド
	if (pipelines != nullptr) {
		pipeline::bind(g_commandBuffer, pipelineCount, pipelines);
	}

	// メッシュバインド
	if (mesh != nullptr) {
		g_indexCount = mesh::bind(g_commandBuffer, mesh);
	}

	// 描画
	g_commandBuffer.drawIndexed(g_indexCount, instanceCount, 0, 0, instanceCount);
}

void endRender(const vk::Device &device, const vk::Queue &queue) {
	// レンダーパス終了
	g_commandBuffer.endRenderPass();

	// コマンドバッファ終了
	g_commandBuffer.end();

	// 提出
	const vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	const auto si = vk::SubmitInfo()
		.setWaitSemaphores({g_semaphoreForImageEnabled})
		.setWaitDstStageMask({waitStage})
		.setCommandBuffers({g_commandBuffer})
		.setSignalSemaphores({g_semaphoreForRenderFinisheds.at(g_index)});
	queue.submit(si, g_frameInFlightFence);

	// プレゼンテーション
	//
	// NOTE: ここで画面が切り替わるまで待機される。
	swapchain::presentation(queue, g_semaphoreForRenderFinisheds.at(g_index), g_index);
}

void terminate(const vk::Device &device) {
	pipeline::terminate(device);

	if (g_frameInFlightFence) {
		device.destroyFence(g_frameInFlightFence);
		g_frameInFlightFence = nullptr;
	}

	for (const auto &semaphore: g_semaphoreForRenderFinisheds) {
	device.destroySemaphore(semaphore);
	}
	g_semaphoreForRenderFinisheds.clear();

	if (g_semaphoreForImageEnabled) {
		device.destroySemaphore(g_semaphoreForImageEnabled);
		g_semaphoreForImageEnabled = nullptr;
	}

	if (g_commandBuffer) {
		// NOTE: コマンドプール全体を破棄するので個別に解放する必要はない。
		g_commandBuffer = nullptr;
	}

	for (auto &framebuffer: g_framebuffers) {
		device.destroyFramebuffer(framebuffer);
	}
	g_framebuffers.clear();

	if (g_renderPass) {
		device.destroyRenderPass(g_renderPass);
		g_renderPass = nullptr;
	}

	g_clearValues.clear();
}

} // namespace graphics::rendering
