#include "image.hpp"

#include "utils.hpp"

#include <format>
#include <memory>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace graphics {

vk::Image createImage(const vk::Device &device, uint32_t width, uint32_t height) {
	const auto ci = vk::ImageCreateInfo()
		.setImageType(vk::ImageType::e2D)
		.setFormat(vk::Format::eR8G8B8A8Srgb)
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
		vk::Format::eR8G8B8A8Srgb,
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
	const uint8_t *pixels
) :
	_image(createImage(device, width, height)),
	_memory(allocateImageMemory(memoryProps, device, _image, vk::MemoryPropertyFlagBits::eHostCoherent)),
	_view(createImageView(device, _image))
{
	uploadImage(memoryProps, device, queue, _image, width, height, pixels);
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
