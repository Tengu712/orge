#include "rendering.hpp"

#include "../platform.hpp"
#include "framebuffer/framebuffer.hpp"
#include "mesh/mesh.hpp"
#include "pipeline/pipeline.hpp"
#include "swapchain/swapchain.hpp"

namespace graphics::rendering {

vk::RenderPass g_renderPass;
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
// 現在のパイプライン
std::string g_pipelineId;
// 現在のメッシュ
std::string g_meshId;

void terminate(const vk::Instance &instance, const vk::Device &device) {
	mesh::terminate(device);

	pipeline::terminate(device);
	framebuffer::terminate(device);

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

	if (g_renderPass) {
		device.destroyRenderPass(g_renderPass);
		g_renderPass = nullptr;
	}

	swapchain::terminate(instance, device);
}

void createRenderPass(const config::Config &config, const vk::Device &device) {
	// アタッチメント
	std::vector<vk::AttachmentDescription> attachments;
	for (const auto &n: config.attachments) {
		attachments.emplace_back(
			vk::AttachmentDescriptionFlags(),
			n.format == config::Format::RenderTarget
				? platformRenderTargetPixelFormat()
				: n.format == config::Format::DepthBuffer
				? vk::Format::eD32Sfloat
				: n.format == config::Format::ShareColorAttachment
				? platformRenderTargetPixelFormat()
				: throw,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			n.discard
				? vk::AttachmentStoreOp::eNone
				: vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			n.format == config::Format::RenderTarget
				? vk::ImageLayout::ePresentSrcKHR
				: n.format == config::Format::DepthBuffer
				? vk::ImageLayout::eDepthStencilAttachmentOptimal
				: n.format == config::Format::ShareColorAttachment
				? vk::ImageLayout::eShaderReadOnlyOptimal
				: throw
		);
	}

	// サブパス & サブパス依存
	std::vector<std::vector<vk::AttachmentReference>> inputss;
	std::vector<std::vector<vk::AttachmentReference>> outputss;
	std::vector<vk::AttachmentReference> depths;
	std::vector<vk::SubpassDescription> subpasses;
	std::vector<vk::SubpassDependency> dependencies;
	for (const auto &n: config.subpasses) {
		std::vector<vk::AttachmentReference> inputs;
		for (const auto &m: n.inputs) {
			inputs.emplace_back(
				config.attachmentMap.at(m.id),
				m.layout == config::InputLayout::DepthStencilReadOnly
					? vk::ImageLayout::eDepthStencilReadOnlyOptimal
					: m.layout == config::InputLayout::ShaderReadOnly
					? vk::ImageLayout::eShaderReadOnlyOptimal
					: throw
			);
		}
		inputss.push_back(std::move(inputs));

		std::vector<vk::AttachmentReference> outputs;
		for (const auto &m: n.outputs) {
			outputs.emplace_back(config.attachmentMap.at(m), vk::ImageLayout::eColorAttachmentOptimal);
		}
		outputss.push_back(std::move(outputs));

		if (n.depth) {
			depths.emplace_back(
				config.attachmentMap.at(n.depth->id),
				n.depth->readOnly
					? vk::ImageLayout::eDepthStencilReadOnlyOptimal
					: vk::ImageLayout::eDepthStencilAttachmentOptimal
			);
		}

		subpasses.emplace_back(
			vk::SubpassDescription()
				.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
				.setInputAttachments(inputss.back())
				.setColorAttachments(outputss.back())
				.setPDepthStencilAttachment(n.depth ? &depths.back() : nullptr)
		);

		for (const auto &m: n.depends) {
			dependencies.emplace_back(
				config.subpassMap.at(m),
				config.subpassMap.at(n.id),
				vk::PipelineStageFlagBits::eAllCommands,
				vk::PipelineStageFlagBits::eAllCommands,
				vk::AccessFlagBits::eMemoryWrite,
				vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite
			);
		}
	}

	// 作成
	const auto ci = vk::RenderPassCreateInfo()
		.setAttachments(attachments)
		.setSubpasses(subpasses)
		.setDependencies(dependencies);
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
	const auto imageCount = swapchain::getImages().size();
	g_semaphoreForImageEnabled = device.createSemaphore({});
	g_semaphoreForRenderFinisheds.reserve(imageCount);
	for (uint32_t i = 0; i < imageCount; ++i) {
		g_semaphoreForRenderFinisheds.push_back(device.createSemaphore({}));
	}
}

void createFence(const vk::Device &device) {
	g_frameInFlightFence = device.createFence({vk::FenceCreateFlagBits::eSignaled});
}

void initialize(
	const config::Config &config,
	const vk::Instance &instance,
	const vk::PhysicalDevice &physicalDevice,
	const vk::Device &device,
	const vk::CommandPool &commandPool
) {
	swapchain::initialize(config, instance, physicalDevice, device);

	createRenderPass(config, device);
	createCommandBuffer(device, commandPool);
	createSemaphores(device);
	createFence(device);

	framebuffer::initialize(
		config,
		physicalDevice.getMemoryProperties(),
		device,
		g_renderPass,
		swapchain::getImageSize(),
		swapchain::getImages()
	);
	pipeline::initialize(config, device, commandPool, g_renderPass);
}

void beginRender(const vk::Device &device) {
	// 前のフレームのGPU処理が完全に終了するまで待機
	if (device.waitForFences({g_frameInFlightFence}, VK_TRUE, UINT64_MAX) != vk::Result::eSuccess) {
		throw "failed to wait for rendering comletion.";
	}
	device.resetFences({g_frameInFlightFence});

	// パイプラインとメッシュのバインド状態をリセット
	g_pipelineId.clear();
	g_meshId.clear();

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
		.setFramebuffer(framebuffer::getFramebuffer(g_index))
		.setRenderArea(vk::Rect2D({0, 0}, swapchain::getImageSize()))
		.setClearValues(framebuffer::getClearValues());
	g_commandBuffer.beginRenderPass(rbi, vk::SubpassContents::eInline);
}

void endRender(const vk::Queue &queue) {
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

void draw(
	const vk::Device &device,
	const char *pipelineId,
	const char *meshId,
	uint32_t instanceCount,
	uint32_t instanceOffset
) {
	if (pipelineId != nullptr && pipelineId != g_pipelineId) {
		pipeline::bind(device, g_commandBuffer, g_index, pipelineId);
		g_pipelineId = pipelineId;
	}
	if (meshId != nullptr && meshId != g_meshId) {
		g_indexCount = mesh::bind(g_commandBuffer, meshId);
		g_meshId = meshId;
	}
	g_commandBuffer.drawIndexed(g_indexCount, instanceCount, 0, 0, instanceOffset);
}

} // namespace graphics::rendering

#include "../../error/error.hpp"

int orgeBindDescriptorSets(const char *id, uint32_t const *indices) {
	TRY(graphics::rendering::pipeline::bindDescriptorSets(graphics::rendering::g_commandBuffer, id, indices));
}

void orgeNextSubpass() {
	graphics::rendering::g_commandBuffer.nextSubpass(vk::SubpassContents::eInline);
}
