#include "image.hpp"

#include "../core/core.hpp"
#include "../utils.hpp"

namespace graphics::resource {

vk::Image createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageUsageFlags usage) {
	const auto ci = vk::ImageCreateInfo()
		.setImageType(vk::ImageType::e2D)
		.setFormat(format)
		.setExtent(vk::Extent3D(width, height, 1))
		.setMipLevels(1)
		.setArrayLayers(1)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setTiling(vk::ImageTiling::eOptimal)
		.setUsage(usage)
		.setSharingMode(vk::SharingMode::eExclusive);
	return core::device().createImage(ci);
}

vk::ImageView createImageView(const vk::Image &image, vk::Format format, vk::ImageAspectFlags aspect) {
	const auto vci = vk::ImageViewCreateInfo(
		vk::ImageViewCreateFlags(),
		image,
		vk::ImageViewType::e2D,
		format,
		vk::ComponentMapping(
			vk::ComponentSwizzle::eR,
			vk::ComponentSwizzle::eG,
			vk::ComponentSwizzle::eB,
			vk::ComponentSwizzle::eA
		),
		vk::ImageSubresourceRange(aspect, 0, 1, 0, 1)
	);
	return core::device().createImageView(vci);
}

Image::Image(
	const vk::Image &image,
	vk::Format format,
	vk::ImageAspectFlags aspect,
	uint32_t chCount
):
	_image(image),
	_memory(nullptr),
	_view(createImageView(_image, format, aspect)),
	_chCount(chCount)
{}

Image::Image(
	uint32_t width,
	uint32_t height,
	const uint8_t *pixels,
	vk::Format format,
	vk::ImageUsageFlags usage,
	vk::ImageAspectFlags aspect,
	uint32_t chCount
):
	_image(createImage(width, height, format, usage)),
	_memory(allocateImageMemory(_image, vk::MemoryPropertyFlagBits::eDeviceLocal)),
	_view(createImageView(_image, format, aspect)),
	_chCount(chCount)
{
	if (pixels) {
		uploadImage(_image, width, height, chCount, 0, 0, pixels);
	}
}

Image::~Image() {
	if (_memory) {
		core::device().free(_memory);
	}
	core::device().destroy(_view);
	if (_memory) {
		core::device().destroy(_image);
	}
}

void Image::upload(
	uint32_t width,
	uint32_t height,
	uint32_t offsetX,
	uint32_t offsetY,
	const uint8_t *src
) {
	uploadImage(_image, width, height, _chCount, offsetX, offsetY, src);
}

} // namespace graphics::resource
