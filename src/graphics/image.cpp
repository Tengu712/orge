#include "image.hpp"

#include "utils.hpp"

#include <format>
#include <memory>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace graphics {

vk::Image createImage(const vk::Device &device, uint32_t width, uint32_t height, bool charAtlus) {
	const auto ci = vk::ImageCreateInfo()
		.setImageType(vk::ImageType::e2D)
		.setFormat(charAtlus ? vk::Format::eR8Unorm : vk::Format::eR8G8B8A8Srgb)
		.setExtent(vk::Extent3D(width, height, 1))
		.setMipLevels(1)
		.setArrayLayers(1)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setTiling(vk::ImageTiling::eOptimal)
		.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst)
		.setSharingMode(vk::SharingMode::eExclusive);
	return device.createImage(ci);
}

vk::ImageView createImageView(const vk::Device &device, const vk::Image &image, bool charAtlus) {
	const auto vci = vk::ImageViewCreateInfo(
		vk::ImageViewCreateFlags(),
		image,
		vk::ImageViewType::e2D,
		charAtlus ? vk::Format::eR8Unorm : vk::Format::eR8G8B8A8Srgb,
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
	const uint8_t *pixels,
	bool charAtlus
) :
	_image(createImage(device, width, height, charAtlus)),
	_memory(allocateImageMemory(memoryProps, device, _image, vk::MemoryPropertyFlagBits::eDeviceLocal)),
	_view(createImageView(device, _image, charAtlus))
{
	uploadImage(memoryProps, device, queue, _image, width, height, charAtlus ? 1 : 4, 0, 0, pixels);
}

Image Image::fromFile(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Queue &queue,
	const std::string &path
) {
	using stbi_ptr = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>;

	int width = 0;
	int height = 0;
	int channelCount = 0;
	const auto pixels = stbi_ptr(
		stbi_load(path.c_str(), &width, &height, &channelCount, 0),
		stbi_image_free
	);
	if (!pixels) {
		throw std::format("failed to load '{}'.", path);
	}
	if (channelCount != 4) {
		throw std::format("'{}' is not RGBA.", path);
	}

	return Image(memoryProps, device, queue, width, height, pixels.get(), false);
}

void Image::upload(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Queue &queue,
	uint32_t width,
	uint32_t height,
	uint32_t offsetX,
	uint32_t offsetY,
	const uint8_t *src,
	bool charAtlus
) {
	uploadImage(
		memoryProps,
		device,
		queue,
		_image,
		width,
		height,
		charAtlus ? 1 : 4,
		offsetX,
		offsetY,
		src
	);
}

} // namespace graphics
