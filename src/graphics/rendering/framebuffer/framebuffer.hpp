#pragma once

#include "../../../config/config.hpp"

#include <vulkan/vulkan.hpp>

namespace graphics::rendering::framebuffer {

struct Attachment {
	const vk::Image image;
	const vk::ImageView view;
	const vk::DeviceMemory memory;
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
		const vk::RenderPass &renderPass,
		const vk::Image &swapchainImage,
		const vk::Extent2D &swapchainImageExtent
	);

	void destroy(const vk::Device &device) const noexcept {
		for (auto &n: _attachments) {
			if (n.memory) {
				device.freeMemory(n.memory);
			}
			device.destroyImageView(n.view);
			if (n.image) {
				device.destroyImage(n.image);
			}
		}
		device.destroy(_framebuffer);
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

} // namespace graphics::rendering::framebuffer
