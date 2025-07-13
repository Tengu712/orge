#include "image.hpp"

#include "../utils.hpp"

#include <format>
#include <memory>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <unordered_map>

namespace graphics::pipeline::image {

vk::CommandBuffer g_commandBuffer;
vk::Fence g_fence;
std::unordered_map<std::string, Image> g_images;
std::unordered_map<uint8_t, vk::Sampler> g_samplers;

void terminate(const vk::Device &device) {
	for (auto &n: g_samplers) {
		device.destroySampler(n.second);
	}
	g_samplers.clear();

	for (auto &n: g_images) {
		device.freeMemory(n.second.memory);
		device.destroyImageView(n.second.view);
		device.destroyImage(n.second.image);
	}
	g_images.clear();

	if (g_fence) {
		device.destroyFence(g_fence);
		g_fence = nullptr;
	}

	// NOTE: コマンドプール全体を破棄するので個別に解放する必要はない。
	g_commandBuffer = nullptr;
}

void initialize(const vk::Device &device, const vk::CommandPool &commandPool) {
	const auto ai = vk::CommandBufferAllocateInfo()
		.setCommandPool(commandPool)
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1);
	g_commandBuffer = device.allocateCommandBuffers(ai).at(0);

	g_fence = device.createFence({});
}

inline uint8_t makeSamplerKey(int linearMagFilter, int linearMinFilter, int repeat) {
    return static_cast<bool>(linearMagFilter)
		| (static_cast<bool>(linearMinFilter) << 1)
		| (static_cast<bool>(repeat) << 2);
}

const vk::Sampler &createSampler(const vk::Device &device, int linearMagFilter, int linearMinFilter, int repeat) {
	const auto key = makeSamplerKey(linearMagFilter, linearMinFilter, repeat);

	if (!g_samplers.contains(key)) {
		const auto ci = vk::SamplerCreateInfo()
			.setMagFilter(linearMagFilter ? vk::Filter::eLinear : vk::Filter::eNearest)
			.setMinFilter(linearMinFilter ? vk::Filter::eLinear : vk::Filter::eNearest)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear)
			.setAddressModeU(repeat ? vk::SamplerAddressMode::eRepeat : vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeV(repeat ? vk::SamplerAddressMode::eRepeat : vk::SamplerAddressMode::eClampToEdge)
			.setMaxLod(vk::LodClampNone);
		const auto sampler = device.createSampler(ci);
		g_samplers.emplace(key, sampler);
	}

	return g_samplers.at(key);
}

void create(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Queue &queue,
	const char *id,
	uint32_t width,
	uint32_t height,
	const unsigned char *pixels,
	int linearMagFilter,
	int linearMinFilter,
	int repeat
) {
	const auto extent = vk::Extent3D(width, height, 1);

	// イメージ作成
	const auto ci = vk::ImageCreateInfo()
		.setImageType(vk::ImageType::e2D)
		.setFormat(vk::Format::eR8G8B8A8Unorm)
		.setExtent(extent)
		.setMipLevels(1)
		.setArrayLayers(1)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setTiling(vk::ImageTiling::eOptimal)
		.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst)
		.setSharingMode(vk::SharingMode::eExclusive);
	const auto image = device.createImage(ci);

	// メモリ確保
	const auto memory = utils::allocateImageMemory(
		memoryProps,
		device,
		image,
		vk::MemoryPropertyFlagBits::eHostCoherent
	);

	// イメージビュー作成
	const auto subresRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
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
		subresRange
	);
	const auto view = device.createImageView(vci);

	// ステージングバッファ作成
	const auto bufferSize = width * height * 4;
	const auto bci = vk::BufferCreateInfo()
		.setSize(bufferSize)
		.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
		.setSharingMode(vk::SharingMode::eExclusive);
	const auto buffer = device.createBuffer(bci);
	const auto bufferMemory = utils::allocateMemory(
		memoryProps,
		device,
		buffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);

	// ステージングバッファへアップロード
	const auto p = device.mapMemory(bufferMemory, 0, bufferSize);
	memcpy(p, pixels, bufferSize);
	device.unmapMemory(bufferMemory);

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
		image,
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
	g_commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, {cr});

	// メモリバリア (transferDstOptimal -> shaderReadOnlyOptimal)
	const auto amb = vk::ImageMemoryBarrier(
		vk::AccessFlagBits::eTransferWrite,
		vk::AccessFlagBits::eShaderRead,
		vk::ImageLayout::eTransferDstOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::QueueFamilyIgnored,
		vk::QueueFamilyIgnored,
		image,
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

	// サンプラ作成
	const auto &sampler = createSampler(device, linearMagFilter, linearMinFilter, repeat);

	// 終了
	g_images.emplace(id, Image{image, view, memory, sampler});
}

void createFromFile(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Queue &queue,
	const char *id,
	const char *path,
	int linearMagFilter,
	int linearMinFilter,
	int repeat
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

	create(memoryProps, device, queue, id, width, height, pixels.get(), linearMagFilter, linearMinFilter, repeat);
}

const Image &get(const char *id) {
	return g_images.at(id);
}

} // namespace graphics::pipeline::image
