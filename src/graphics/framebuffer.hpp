#pragma once

#include "../config/config.hpp"
#include "swapchain.hpp"

#include <vulkan/vulkan.hpp>

namespace graphics {

struct Attachment {
	const vk::Image image;
	const vk::ImageView view;
	const vk::DeviceMemory memory;

	Attachment() = delete;
	Attachment(const vk::Image &image, const vk::ImageView &view, const vk::DeviceMemory &memory):
		image(image), view(view), memory(memory)
	{}

	void destroy(const vk::Device &device) const noexcept {
		if (memory) {
			device.freeMemory(memory);
		}
		device.destroyImageView(view);
		if (image) {
			device.destroyImage(image);
		}
	}
};

class Framebuffer {
private:
	const std::vector<Attachment> _attachments;
	const std::vector<vk::ClearValue> _clearValues;
	const vk::Framebuffer _framebuffer;

public:
	Framebuffer() = delete;
	Framebuffer(
		const config::Config &config,
		const vk::PhysicalDeviceMemoryProperties &memoryProps,
		const vk::Device &device,
		const Swapchain &swapchain,
		const vk::Image &swapchainImage,
		const vk::RenderPass &renderPass
	);

	void destroy(const vk::Device &device) const noexcept {
		device.destroy(_framebuffer);
		for (auto &n: _attachments) {
			n.destroy(device);
		}
	}

	const vk::Framebuffer &get() const noexcept {
		return _framebuffer;
	}

	const vk::ImageView &getAttachmentView(uint32_t index) const {
		return _attachments.at(index).view;
	}

	const std::vector<vk::ClearValue> &getClearValues() const noexcept {
		return _clearValues;
	}
};

} // namespace graphics
