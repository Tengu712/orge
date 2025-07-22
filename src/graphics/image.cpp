#include "image.hpp"

#include "utils.hpp"

#include <format>
#include <memory>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace graphics {

vk::CommandBuffer g_commandBuffer;
vk::Fence g_fence;

void createCommandBufferForCreatingImage(const vk::Device &device, const vk::CommandPool &commandPool) {
	const auto ai = vk::CommandBufferAllocateInfo()
		.setCommandPool(commandPool)
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1);
	g_commandBuffer = device.allocateCommandBuffers(ai).at(0);

	g_fence = device.createFence({});
}

void destroyCommandBufferForCreatingImage(const vk::Device &device) {
	if (g_fence) {
		device.destroyFence(g_fence);
		g_fence = nullptr;
	}

	// NOTE: コマンドプール全体を破棄するので個別に解放する必要はない。
	g_commandBuffer = nullptr;
}

vk::Image createImage(const vk::Device &device, uint32_t width, uint32_t height) {
	const auto ci = vk::ImageCreateInfo()
		.setImageType(vk::ImageType::e2D)
		.setFormat(vk::Format::eR8G8B8A8Unorm)
		.setExtent(vk::Extent3D(width, height, 1))
		.setMipLevels(1)
		.setArrayLayers(1)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setTiling(vk::ImageTiling::eOptimal)
		.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst)
		.setSharingMode(vk::SharingMode::eExclusive);
	return device.createImage(ci);
}

vk::ImageView createImageView(const vk::Device &device, const vk::Image &image) {
	const auto vci = vk::ImageViewCreateInfo(
		vk::ImageViewCreateFlags(),
		image,
		vk::ImageViewType::e2D,
		vk::Format::eR8G8B8A8Unorm,
		vk::ComponentMapping(
			vk::ComponentSwizzle::eR,
			vk::ComponentSwizzle::eG,
			vk::ComponentSwizzle::eB,
			vk::ComponentSwizzle::eA
		),
		vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
	);
	return device.createImageView(vci);
}

Image::Image(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Queue &queue,
	uint32_t width,
	uint32_t height,
	const unsigned char *pixels
) :
	_image(createImage(device, width, height)),
	_memory(allocateImageMemory(memoryProps, device, _image, vk::MemoryPropertyFlagBits::eHostCoherent)),
	_view(createImageView(device, _image))
{
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
	copyDataToMemory(device, bufferMemory, pixels, bufferSize);

	// アップロード準備
	device.resetFences({g_fence});
	g_commandBuffer.reset();
	const auto cbi = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	g_commandBuffer.begin(cbi);
	const auto extent = vk::Extent3D(width, height, 1);
	const auto subresRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

	// メモリバリア (undegined -> transferDstOptimal)
	const auto bmb = vk::ImageMemoryBarrier(
		vk::AccessFlags(),
		vk::AccessFlagBits::eTransferWrite,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eTransferDstOptimal,
		vk::QueueFamilyIgnored,
		vk::QueueFamilyIgnored,
		_image,
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
	g_commandBuffer.copyBufferToImage(buffer, _image, vk::ImageLayout::eTransferDstOptimal, {cr});

	// メモリバリア (transferDstOptimal -> shaderReadOnlyOptimal)
	const auto amb = vk::ImageMemoryBarrier(
		vk::AccessFlagBits::eTransferWrite,
		vk::AccessFlagBits::eShaderRead,
		vk::ImageLayout::eTransferDstOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::QueueFamilyIgnored,
		vk::QueueFamilyIgnored,
		_image,
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

Image Image::fromFile(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Queue &queue,
	const char *path
) {
	using stbi_ptr = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>;

	int width = 0;
	int height = 0;
	int channelCount = 0;
	const auto pixels = stbi_ptr(
		stbi_load(path, &width, &height, &channelCount, 0),
		stbi_image_free
	);
	if (!pixels) {
		throw std::format("failed to load '{}'.", path);
	}
	if (channelCount != 4) {
		throw std::format("'{}' is not RGBA.", path);
	}

	return Image(memoryProps, device, queue, width, height, pixels.get());
}

} // namespace graphics
