#include "utils.hpp"

#include <optional>

namespace graphics {

std::optional<vk::UniqueCommandBuffer> g_commandBuffer;
std::optional<vk::UniqueFence> g_fence;

void terminateUtils() {
	if (g_fence) {
		g_fence.reset();
	}
	if (g_commandBuffer) {
		// NOTE: コマンドプール全体を破棄するので個別に解放する必要はない。
		g_commandBuffer.reset();
	}
}

void initializeUtils() {
	const auto ai = vk::CommandBufferAllocateInfo()
		.setCommandPool(core::commandPool())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1);
	auto commandBuffers = core::device().allocateCommandBuffersUnique(ai);
	if (commandBuffers.empty()) {
		throw "failed to allocate a command buffer for utility.";
	}
	g_commandBuffer = std::move(commandBuffers[0]);

	g_fence = core::device().createFenceUnique({});
}

void uploadBuffer(const vk::Buffer &dst, const void *src, size_t size, vk::PipelineStageFlags visibleStages) {
	const auto &device = core::device();
	const auto &fence = g_fence.value();
	const auto &commandBuffer = g_commandBuffer.value();

	// ステージングバッファ作成
	const auto bci = vk::BufferCreateInfo()
		.setSize(static_cast<vk::DeviceSize>(size))
		.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
		.setSharingMode(vk::SharingMode::eExclusive);
	const auto buffer = device.createBufferUnique(bci);
	const auto bufferMemory = allocateMemory(buffer.get(), vk::MemoryPropertyFlagBits::eHostVisible);

	// ステージングバッファへアップロード
	copyDataToMemory(bufferMemory.get(), src, size);

	// アップロード準備
	commandBuffer->reset();
	const auto cbi = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	commandBuffer->begin(cbi);

	// メモリバリア (undegined -> transferDstOptimal)
	const auto bmb = vk::BufferMemoryBarrier(
		vk::AccessFlags(),
		vk::AccessFlagBits::eTransferWrite,
		vk::QueueFamilyIgnored,
		vk::QueueFamilyIgnored,
		dst,
		0,
		size
	);
	commandBuffer->pipelineBarrier(
		vk::PipelineStageFlagBits::eAllCommands,
		vk::PipelineStageFlagBits::eTransfer,
		vk::DependencyFlags(),
		{},
		{bmb},
		{}
	);

	// アップロード
	const auto cr = vk::BufferCopy(0, 0, size);
	commandBuffer->copyBuffer(buffer.get(), dst, {cr});

	// メモリバリア (transferDstOptimal -> shaderReadOnlyOptimal)
	const auto amb = vk::BufferMemoryBarrier(
		vk::AccessFlagBits::eTransferWrite,
		vk::AccessFlagBits::eShaderRead,
		vk::QueueFamilyIgnored,
		vk::QueueFamilyIgnored,
		dst,
		0,
		size
	);
	commandBuffer->pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer,
		visibleStages,
		vk::DependencyFlags(),
		{},
		{amb},
		{}
	);

	// アップロード提出
	commandBuffer->end();
	device.resetFences({fence.get()});
	const auto si = vk::SubmitInfo()
		.setCommandBuffers({commandBuffer.get()});
	core::queue().submit(si, fence.get());

	// 完了まで待機
	if (device.waitForFences({fence.get()}, VK_TRUE, UINT64_MAX) != vk::Result::eSuccess) {
		throw "failed to wait for uploading a buffer.";
	}
}

void uploadImage(
	const vk::Image &dst,
	uint32_t width,
	uint32_t height,
	uint32_t channels,
	uint32_t offsetX,
	uint32_t offsetY,
	const uint8_t *src
) {
	const auto &device = core::device();
	const auto &fence = g_fence.value();
	const auto &commandBuffer = g_commandBuffer.value();
	const auto extent = vk::Extent3D(width, height, 1);
	const auto subresRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

	// ステージングバッファ作成
	const auto bufferSize = width * height * channels;
	const auto bci = vk::BufferCreateInfo()
		.setSize(bufferSize)
		.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
		.setSharingMode(vk::SharingMode::eExclusive);
	const auto buffer = device.createBufferUnique(bci);
	const auto bufferMemory = allocateMemory(buffer.get(), vk::MemoryPropertyFlagBits::eHostVisible);

	// ステージングバッファへアップロード
	copyDataToMemory(bufferMemory.get(), src, bufferSize);

	// アップロード準備
	commandBuffer->reset();
	const auto cbi = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	commandBuffer->begin(cbi);

	// メモリバリア (undegined -> transferDstOptimal)
	const auto bmb = vk::ImageMemoryBarrier(
		vk::AccessFlags(),
		vk::AccessFlagBits::eTransferWrite,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eTransferDstOptimal,
		vk::QueueFamilyIgnored,
		vk::QueueFamilyIgnored,
		dst,
		subresRange
	);
	commandBuffer->pipelineBarrier(
		vk::PipelineStageFlagBits::eAllCommands,
		vk::PipelineStageFlagBits::eTransfer,
		vk::DependencyFlags(),
		{},
		{},
		{bmb}
	);

	// アップロード
	const auto cr = vk::BufferImageCopy()
		.setImageSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1))
		.setImageOffset(vk::Offset3D(offsetX, offsetY, 0))
		.setImageExtent(extent);
	commandBuffer->copyBufferToImage(buffer.get(), dst, vk::ImageLayout::eTransferDstOptimal, {cr});

	// メモリバリア (transferDstOptimal -> shaderReadOnlyOptimal)
	const auto amb = vk::ImageMemoryBarrier(
		vk::AccessFlagBits::eTransferWrite,
		vk::AccessFlagBits::eShaderRead,
		vk::ImageLayout::eTransferDstOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::QueueFamilyIgnored,
		vk::QueueFamilyIgnored,
		dst,
		subresRange
	);
	commandBuffer->pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eFragmentShader,
		vk::DependencyFlags(),
		{},
		{},
		{amb}
	);

	// アップロード提出
	commandBuffer->end();
	device.resetFences({fence.get()});
	const auto si = vk::SubmitInfo()
		.setCommandBuffers({commandBuffer.get()});
	core::queue().submit(si, fence.get());

	// 完了まで待機
	if (device.waitForFences({fence.get()}, VK_TRUE, UINT64_MAX) != vk::Result::eSuccess) {
		throw "failed to wait for uploading an image.";
	}
}

} // namespace graphics
