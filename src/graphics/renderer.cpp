#include "renderer.hpp"

namespace graphics {

vk::RenderPass createRenderPass(const config::Config &config, const vk::Device &device, const vk::Format &rtFormat) {
	// アタッチメント
	std::vector<vk::AttachmentDescription> attachments;
	for (const auto &n: config.attachments) {
		attachments.emplace_back(
			vk::AttachmentDescriptionFlags(),
			n.format == config::Format::RenderTarget
				? rtFormat
				: n.format == config::Format::DepthBuffer
				? vk::Format::eD32Sfloat
				: n.format == config::Format::ShareColorAttachment
				? rtFormat
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
			const auto index = config.attachmentMap.at(m);
			const auto &attachment = config.attachments.at(index);
			inputs.emplace_back(
				index,
				attachment.format == config::Format::DepthBuffer
					? vk::ImageLayout::eDepthStencilReadOnlyOptimal
					: attachment.format == config::Format::ShareColorAttachment
					? vk::ImageLayout::eShaderReadOnlyOptimal
					: throw "the attachment format of a subpass input must be 'depth-buffer' or 'share-color-attachment'."
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
	return device.createRenderPass(ci);
}

vk::CommandBuffer createCommandBuffer(const vk::Device &device, const vk::CommandPool &commandPool) {
	const auto ai = vk::CommandBufferAllocateInfo()
		.setCommandPool(commandPool)
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1);
	return device.allocateCommandBuffers(ai).at(0);
}

std::vector<vk::Semaphore> createSemaphores(const vk::Device &device, size_t count) {
	std::vector<vk::Semaphore> semaphores;
	semaphores.reserve(count);
	for (uint32_t i = 0; i < count; ++i) {
		semaphores.push_back(device.createSemaphore({}));
	}
	return semaphores;
}

std::vector<Framebuffer> createFramebuffers(
	const config::Config &config,
	const vk::PhysicalDevice &physicalDevice,
	const vk::Device &device,
	const Swapchain &swapchain,
	const vk::RenderPass &renderPass
) {
	const auto swapchainImages = swapchain.getImages();
	std::vector<Framebuffer> framebuffers;
	framebuffers.reserve(swapchainImages.size());
	for (const auto &n: swapchainImages) {
		framebuffers.emplace_back(
			config,
			physicalDevice.getMemoryProperties(),
			device,
			swapchain,
			n,
			renderPass
		);
	}
	return framebuffers;
}

Renderer::Renderer(
	const config::Config &config,
	const vk::Instance &instance,
	const vk::PhysicalDevice &physicalDevice,
	const vk::Device &device,
	const vk::CommandPool &commandPool
) :
	_swapchain(
		config.title,
		config.width,
		config.height,
		false, // TODO:
		instance,
		physicalDevice,
		device
	),
	_renderPass(createRenderPass(config, device, _swapchain.getFormat())),
	_commandBuffer(createCommandBuffer(device, commandPool)),
	_semaphoreForImageEnabled(device.createSemaphore({})),
	_semaphoreForRenderFinisheds(createSemaphores(device, _swapchain.getImages().size())),
	_frameInFlightFence(device.createFence({vk::FenceCreateFlagBits::eSignaled})),
	_framebuffers(createFramebuffers(config, physicalDevice, device, _swapchain, _renderPass)),
	_descPool(createDescriptorPool(config, device)),
	_pipelines(createPipelines(config, device, _renderPass, _descPool, _swapchain.getExtent()))
{}

void Renderer::beginRender(const vk::Device &device) {
	// TODO: フレーム情報が残っている場合、前のフレームで描画に失敗したことを意味する。
	// フレーム情報をリセット
	_frameInfo = std::nullopt;

	// コマンドバッファ開始
	_commandBuffer.reset();
	const auto cbi = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	_commandBuffer.begin(cbi);

	// スワップチェインイメージ番号取得
	const auto index = _swapchain.acquireNextImageIndex(device, _semaphoreForImageEnabled);

	// レンダーパス開始
	const auto &framebuffer = _framebuffers.at(index);
	const auto rbi = vk::RenderPassBeginInfo()
		.setRenderPass(_renderPass)
		.setFramebuffer(framebuffer.get())
		.setRenderArea(vk::Rect2D({0, 0}, _swapchain.getExtent()))
		.setClearValues(framebuffer.getClearValues());
	_commandBuffer.beginRenderPass(rbi, vk::SubpassContents::eInline);

	// フレーム情報格納
	_frameInfo.emplace(FrameInfo{index, "", 0, ""});
}

void Renderer::endRender(const vk::Device &device, const vk::Queue &queue) {
	_ensureWhileRendering("try to end rendering before the rendering started.");

	// 終了
	_commandBuffer.endRenderPass();
	_commandBuffer.end();

	// フェンスのリセット
	device.resetFences({_frameInFlightFence});

	// 提出
	// TODO: 提出に失敗するとフェンスがシグナルされない？
	const vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	const auto si = vk::SubmitInfo()
		.setWaitSemaphores({_semaphoreForImageEnabled})
		.setWaitDstStageMask({waitStage})
		.setCommandBuffers({_commandBuffer})
		.setSignalSemaphores({_semaphoreForRenderFinisheds.at(_frameInfo->index)});
	queue.submit(si, _frameInFlightFence);

	// プレゼンテーション
	_swapchain.present(queue, _semaphoreForRenderFinisheds.at(_frameInfo->index), _frameInfo->index);

	// 前のフレームのGPU処理が完全に終了するまで待機
	if (device.waitForFences({_frameInFlightFence}, VK_TRUE, UINT64_MAX) != vk::Result::eSuccess) {
		throw "failed to wait for rendering comletion.";
	}

	// フレーム情報をリセット
	_frameInfo = std::nullopt;
}

void Renderer::recreateSwapchain(
	const config::Config &config,
	const vk::PhysicalDevice &physicalDevice,
	const vk::Device &device
) {
	_destroyForRecreatingSwapchainOrSurface(device);
	_swapchain.recreateSwapchain(physicalDevice, device);
	_framebuffers = createFramebuffers(config, physicalDevice, device, _swapchain, _renderPass);
	_descPool = createDescriptorPool(config, device);
	_pipelines = createPipelines(config, device, _renderPass, _descPool, _swapchain.getExtent());
}

void Renderer::recreateSurface(
	const config::Config &config,
	const vk::Instance &instance,
	const vk::PhysicalDevice &physicalDevice,
	const vk::Device &device
) {
	_destroyForRecreatingSwapchainOrSurface(device);
	_swapchain.recreateSurface(instance, physicalDevice, device);
	_framebuffers = createFramebuffers(config, physicalDevice, device, _swapchain, _renderPass);
	_descPool = createDescriptorPool(config, device);
	_pipelines = createPipelines(config, device, _renderPass, _descPool, _swapchain.getExtent());
}

} // namespace graphics
