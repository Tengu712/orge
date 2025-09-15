#include "utils.hpp"

#include "../error/error.hpp"

namespace graphics {

vk::CommandBuffer g_commandBuffer;
vk::Fence g_fence;

void terminateUtils() {
	if (g_fence) {
		core::device().destroyFence(g_fence);
		g_fence = nullptr;
	}

	// NOTE: コマンドプール全体を破棄するので個別に解放する必要はない。
	g_commandBuffer = nullptr;
}

void initializeUtils() {
	const auto ai = vk::CommandBufferAllocateInfo()
		.setCommandPool(core::commandPool())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1);
	g_commandBuffer = error::at(core::device().allocateCommandBuffers(ai), 0, "command buffers allocated");

	g_fence = core::device().createFence({});
}

void uploadBuffer(const vk::Buffer &dst, const void *src, size_t size, vk::PipelineStageFlags visibleStages) {
	const auto &device = core::device();

	// ステージングバッファ作成
	const auto bci = vk::BufferCreateInfo()
		.setSize(static_cast<vk::DeviceSize>(size))
		.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
		.setSharingMode(vk::SharingMode::eExclusive);
	const auto buffer = device.createBuffer(bci);
	const auto bufferMemory = allocateMemory(buffer, vk::MemoryPropertyFlagBits::eHostVisible);

	// ステージングバッファへアップロード
	copyDataToMemory(bufferMemory, src, size);

	// アップロード準備
	g_commandBuffer.reset();
	const auto cbi = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	g_commandBuffer.begin(cbi);

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
	g_commandBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eAllCommands,
		vk::PipelineStageFlagBits::eTransfer,
		vk::DependencyFlags(),
		{},
		{bmb},
		{}
	);

	// アップロード
	const auto cr = vk::BufferCopy(0, 0, size);
	g_commandBuffer.copyBuffer(buffer, dst, {cr});

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
	g_commandBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer,
		visibleStages,
		vk::DependencyFlags(),
		{},
		{amb},
		{}
	);

	// アップロード提出
	g_commandBuffer.end();
	device.resetFences({g_fence});
	const auto si = vk::SubmitInfo()
		.setCommandBuffers({g_commandBuffer});
	core::queue().submit(si, g_fence);

	// 完了まで待機
	if (device.waitForFences({g_fence}, VK_TRUE, UINT64_MAX) != vk::Result::eSuccess) {
		throw "failed to wait for uploading a buffer.";
	}

	// ステージングバッファ削除
	device.free(bufferMemory);
	device.destroy(buffer);
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
	const auto extent = vk::Extent3D(width, height, 1);
	const auto subresRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

	// ステージングバッファ作成
	const auto bufferSize = width * height * channels;
	const auto bci = vk::BufferCreateInfo()
		.setSize(bufferSize)
		.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
		.setSharingMode(vk::SharingMode::eExclusive);
	const auto buffer = device.createBuffer(bci);
	const auto bufferMemory = allocateMemory(buffer, vk::MemoryPropertyFlagBits::eHostVisible);

	// ステージングバッファへアップロード
	copyDataToMemory(bufferMemory, src, bufferSize);

	// アップロード準備
	g_commandBuffer.reset();
	const auto cbi = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	g_commandBuffer.begin(cbi);

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
	g_commandBuffer.pipelineBarrier(
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
	g_commandBuffer.copyBufferToImage(buffer, dst, vk::ImageLayout::eTransferDstOptimal, {cr});

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
	g_commandBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eFragmentShader,
		vk::DependencyFlags(),
		{},
		{},
		{amb}
	);

	// アップロード提出
	g_commandBuffer.end();
	device.resetFences({g_fence});
	const auto si = vk::SubmitInfo()
		.setCommandBuffers({g_commandBuffer});
	core::queue().submit(si, g_fence);

	// 完了まで待機
	if (device.waitForFences({g_fence}, VK_TRUE, UINT64_MAX) != vk::Result::eSuccess) {
		throw "failed to wait for uploading an image.";
	}

	// ステージングバッファ削除
	device.free(bufferMemory);
	device.destroy(buffer);
}

} // namespace graphics
