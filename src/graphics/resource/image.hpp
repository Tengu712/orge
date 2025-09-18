#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics::resource {

class Image {
private:
	const vk::Image _image;
	const vk::DeviceMemory _memory;
	const vk::ImageView _view;
	const uint32_t _chCount;

public:
	Image() = delete;

	Image(
		const vk::Image &image,
		vk::Format format,
		vk::ImageAspectFlags aspect,
		uint32_t chCount
	);
	Image(
		uint32_t width,
		uint32_t height,
		const uint8_t *pixels,
		vk::Format format,
		vk::ImageUsageFlags usage,
		vk::ImageAspectFlags aspect,
		uint32_t chCount
	);
	virtual ~Image();

	const vk::ImageView &get() const noexcept {
		return _view;
	}

	void upload(
		uint32_t width,
		uint32_t height,
		uint32_t offsetX,
		uint32_t offsetY,
		const uint8_t *src
	);
};

} // namespace graphics::resource
