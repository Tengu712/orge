#include "rendering.hpp"

#include "mesh.hpp"
#include "pipeline.hpp"
#include "platform.hpp"
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

void getClearValues(const config::Config &config) {
	for (const auto &n: config.attachments) {
		if (n.colorClearValue) {
			g_clearValues.emplace_back(static_cast<vk::ClearValue>(vk::ClearColorValue(n.colorClearValue.value())));
		} else {
			g_clearValues.emplace_back(static_cast<vk::ClearValue>(vk::ClearDepthStencilValue(n.depthClearValue.value(), 0)));
		}
	}
}

void createRenderPass(
	const config::Config &config,
	const vk::Device &device,
	std::unordered_map<std::string, uint32_t> &subpassMap
) {
	// アタッチメント
	std::unordered_map<std::string, uint32_t> attachmentMap;
	std::vector<vk::AttachmentDescription> attachments;
	for (const auto &n: config.attachments) {
		if (!attachmentMap.emplace(n.id, static_cast<uint32_t>(attachmentMap.size())).second) {
			throw std::format("attachment id '{}' is duplicated.", n.id);
		}

		attachments.emplace_back(
			vk::AttachmentDescriptionFlags(),
			n.format == config::Format::RenderTarget
				? platform::getRenderTargetPixelFormat()
				: throw,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			n.discard
				? vk::AttachmentStoreOp::eNone
				: vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			n.finalLayout == config::FinalLayout::ColorAttachment
				? vk::ImageLayout::eColorAttachmentOptimal
				: n.finalLayout == config::FinalLayout::DepthStencilAttachment
				? vk::ImageLayout::eDepthStencilAttachmentOptimal
				: n.finalLayout == config::FinalLayout::PresentSrc
				? vk::ImageLayout::ePresentSrcKHR
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
		if (!subpassMap.emplace(n.id, static_cast<uint32_t>(subpassMap.size())).second) {
			throw std::format("subpass id '{}' is duplicated.", n.id);
		}

		std::vector<vk::AttachmentReference> inputs;
		for (const auto &m: n.inputs) {
			if (!attachmentMap.contains(m.id)) {
				throw std::format("attachment '{}' is not defined.", m.id);
			}
			inputs.emplace_back(
				attachmentMap.at(m.id),
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
			if (!attachmentMap.contains(m)) {
				throw std::format("attachment '{}' is not defined.", m);
			}
			outputs.emplace_back(attachmentMap.at(m), vk::ImageLayout::eColorAttachmentOptimal);
		}
		outputss.push_back(std::move(outputs));

		if (n.depth) {
			if (!attachmentMap.contains(n.depth->id)) {
				throw std::format("attachment '{}' is not defined.", n.depth->id);
			}
			depths.emplace_back(
				attachmentMap.at(n.depth->id),
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
			if (!subpassMap.contains(m)) {
				throw std::format("subpass '{}' is not defined.", m);
			}
			dependencies.emplace_back(
				subpassMap.at(m),
				subpassMap.at(n.id),
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
	g_semaphoreForImageEnabled = device.createSemaphore({});
	g_semaphoreForRenderFinisheds.reserve(swapchain::getImageCount());
	for (uint32_t i = 0; i < swapchain::getImageCount(); ++i) {
		g_semaphoreForRenderFinisheds.push_back(device.createSemaphore({}));
	}
}

void createFence(const vk::Device &device) {
	g_frameInFlightFence = device.createFence({vk::FenceCreateFlagBits::eSignaled});
}

void initialize(const config::Config &config, const vk::Device &device, const vk::CommandPool &commandPool) {
	getClearValues(config);
	std::unordered_map<std::string, uint32_t> subpassMap;
	createRenderPass(config, device, subpassMap);
	g_framebuffers = swapchain::createFramebuffers(device, g_renderPass);
	createCommandBuffer(device, commandPool);
	createSemaphores(device);
	createFence(device);
	pipeline::initialize(config, subpassMap, device, g_renderPass);
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
	g_commandBuffer.drawIndexed(g_indexCount, instanceCount, 0, 0, instanceOffset);
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

int orgeBindDescriptorSets(
	const char *id,
	uint32_t count,
	uint32_t const *indices
) {
	TRY(graphics::pipeline::bindDescriptorSets(graphics::rendering::g_commandBuffer, id, count, indices));
}
