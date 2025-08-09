#pragma once

#include <string>
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
		const uint8_t *pixels,
		bool charAtlus
	);

	static Image fromFile(
		const vk::PhysicalDeviceMemoryProperties &memoryProps,
		const vk::Device &device,
		const vk::Queue &queue,
		const std::string &path
	);

	void destroy(const vk::Device &device) const noexcept {
		device.freeMemory(_memory);
		device.destroyImageView(_view);
		device.destroyImage(_image);
	}

	const vk::ImageView &get() const noexcept {
		return _view;
	}

	void upload(
		const vk::PhysicalDeviceMemoryProperties &memoryProps,
		const vk::Device &device,
		const vk::Queue &queue,
		uint32_t width,
		uint32_t height,
		uint32_t offsetX,
		uint32_t offsetY,
		const uint8_t *src,
		bool charAtlus
	);
};

} // namespace graphics
