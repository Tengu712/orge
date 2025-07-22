#include "utils.hpp"

namespace graphics {

vk::CommandBuffer g_commandBuffer;
vk::Fence g_fence;

void terminateUtils(const vk::Device &device) {
	if (g_fence) {
		device.destroyFence(g_fence);
		g_fence = nullptr;
	}

	// NOTE: コマンドプール全体を破棄するので個別に解放する必要はない。
	g_commandBuffer = nullptr;
}

void initializeUtils(const vk::Device &device, const vk::CommandPool &commandPool) {
	const auto ai = vk::CommandBufferAllocateInfo()
		.setCommandPool(commandPool)
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1);
	g_commandBuffer = device.allocateCommandBuffers(ai).at(0);

	g_fence = device.createFence({});
}

void uploadImage(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Queue &queue,
	const vk::Image &dst,
	uint32_t width,
	uint32_t height,
	const unsigned char *src
) {
	const auto extent = vk::Extent3D(width, height, 1);
	const auto subresRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

	// ステージングバッファ作成
	const auto bufferSize = width * height * 4;
	const auto bci = vk::BufferCreateInfo()
		.setSize(bufferSize)
		.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
		.setSharingMode(vk::SharingMode::eExclusive);
	const auto buffer = device.createBuffer(bci);
	const auto bufferMemory = allocateBufferMemory(
		memoryProps,
		device,
		buffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);

	// ステージングバッファへアップロード
	copyDataToMemory(device, bufferMemory, src, bufferSize);

	// アップロード準備
	device.resetFences({g_fence});
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
	const auto si = vk::SubmitInfo()
		.setCommandBuffers({g_commandBuffer});
	queue.submit(si, g_fence);

	// 完了まで待機
	if (device.waitForFences({g_fence}, VK_TRUE, UINT64_MAX) != vk::Result::eSuccess) {
		throw "failed to wait for uploading an image.";
	}

	// ステージングバッファ削除
	device.freeMemory(bufferMemory);
	device.destroyBuffer(buffer);
}

} // namespace graphics
