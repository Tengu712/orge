#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics {

class Image {
private:
	const vk::Image _image;
	const vk::DeviceMemory _memory;
	const vk::ImageView _view;

public:
	Image() = delete;
	Image(
		const vk::PhysicalDeviceMemoryProperties &memoryProps,
		const vk::Device &device,
		const vk::Queue &queue,
		uint32_t width,
		uint32_t height,
		const unsigned char *pixels
	);

	static Image fromFile(
		const vk::PhysicalDeviceMemoryProperties &memoryProps,
		const vk::Device &device,
		const vk::Queue &queue,
		const char *path
	);

	void destroy(const vk::Device &device) const noexcept {
		device.freeMemory(_memory);
		device.destroyImageView(_view);
		device.destroyImage(_image);
	}

	const vk::ImageView &get() const noexcept {
		return _view;
	}
};

} // namespace graphics
