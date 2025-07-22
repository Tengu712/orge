#include "framebuffer.hpp"

#include "../../platform.hpp"
#include "../../utils.hpp"

namespace graphics::rendering::framebuffer {

vk::Format convertFormat(const config::Format &format) {
	return format == config::Format::RenderTarget
		? platformRenderTargetPixelFormat()
		: format == config::Format::DepthBuffer
		? vk::Format::eD32Sfloat
		: format == config::Format::ShareColorAttachment
		? platformRenderTargetPixelFormat()
		: throw;
}

vk::ImageUsageFlags convertUsage(const config::Format &format) {
	return format == config::Format::DepthBuffer
		? vk::ImageUsageFlagBits::eDepthStencilAttachment
		: format == config::Format::ShareColorAttachment
		? vk::ImageUsageFlagBits::eColorAttachment
			| vk::ImageUsageFlagBits::eInputAttachment
			| vk::ImageUsageFlagBits::eSampled
		: throw;
}

vk::ImageAspectFlags convertAspect(const config::Format &format) {
	return format == config::Format::RenderTarget
		? vk::ImageAspectFlagBits::eColor
		: format == config::Format::DepthBuffer
		? vk::ImageAspectFlagBits::eDepth
		: format == config::Format::ShareColorAttachment
		? vk::ImageAspectFlagBits::eColor
		: throw;
}

Attachment createAttachment(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Extent3D &swapchainImageExtent,
	const vk::Image &swapchainImage,
	config::Format format
) {
	const auto isRenderTarget = format == config::Format::RenderTarget;

	// イメージ作成
	vk::Image image = swapchainImage;
	if (!isRenderTarget) {
		const auto ci = vk::ImageCreateInfo()
			.setImageType(vk::ImageType::e2D)
			.setFormat(convertFormat(format))
			.setExtent(swapchainImageExtent)
			.setMipLevels(1)
			.setArrayLayers(1)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal)
			.setUsage(convertUsage(format))
			.setSharingMode(vk::SharingMode::eExclusive);
		image = device.createImage(ci);
	}

	// メモリ確保
	vk::DeviceMemory memory = nullptr;
	if (!isRenderTarget) {
		memory = allocateImageMemory(
			memoryProps,
			device,
			image,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);
	}

	// イメージビュー作成
	const auto subresRange = vk::ImageSubresourceRange(convertAspect(format), 0, 1, 0, 1);
	const auto vci = vk::ImageViewCreateInfo(
		vk::ImageViewCreateFlags(),
		image,
		vk::ImageViewType::e2D,
		convertFormat(format),
		vk::ComponentMapping(
			vk::ComponentSwizzle::eR,
			vk::ComponentSwizzle::eG,
			vk::ComponentSwizzle::eB,
			vk::ComponentSwizzle::eA
		),
		subresRange
	);
	const auto view = device.createImageView(vci);

	// 終了
	return Attachment{isRenderTarget ? nullptr : image, view, memory};
}

std::vector<Attachment> createAttachments(
	const config::Config &config,
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Image &swapchainImage,
	const vk::Extent2D &swapchainImageExtent
) {
	const auto extent = vk::Extent3D(swapchainImageExtent.width, swapchainImageExtent.height, 1);
	std::vector<Attachment> attachments;
	attachments.reserve(config.attachments.size());
	for (const auto &n: config.attachments) {
		attachments.push_back(createAttachment(memoryProps, device, extent, swapchainImage, n.format));
	}
	return attachments;
}

std::vector<vk::ClearValue> collectClearValues(const config::Config &config) {
	std::vector<vk::ClearValue> clearValues;
	clearValues.reserve(config.attachments.size());
	for (const auto &n: config.attachments) {
		if (n.colorClearValue) {
			clearValues.emplace_back(static_cast<vk::ClearValue>(vk::ClearColorValue(n.colorClearValue.value())));
		} else {
			clearValues.emplace_back(static_cast<vk::ClearValue>(vk::ClearDepthStencilValue(n.depthClearValue.value(), 0)));
		}
	}
	return clearValues;
}

vk::Framebuffer createFramebuffer(
	const vk::Device &device,
	const vk::RenderPass &renderPass,
	const vk::Extent2D &swapchainImageExtent,
	const std::vector<Attachment> &attachments
) {
	std::vector<vk::ImageView> as;
	as.reserve(attachments.size());
	for (const auto &n: attachments) {
		as.push_back(n.view);
	}

	const auto ci = vk::FramebufferCreateInfo()
		.setRenderPass(renderPass)
		.setAttachments(as)
		.setWidth(swapchainImageExtent.width)
		.setHeight(swapchainImageExtent.height)
		.setLayers(1);
	return device.createFramebuffer(ci);
}

Framebuffer::Framebuffer(
	const config::Config &config,
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::RenderPass &renderPass,
	const vk::Image &swapchainImage,
	const vk::Extent2D &swapchainImageExtent
) :
	_attachments(createAttachments(config, memoryProps, device, swapchainImage, swapchainImageExtent)),
	_clearValues(collectClearValues(config)),
	_framebuffer(createFramebuffer(device, renderPass, swapchainImageExtent, _attachments))
{}

} // namespace graphics::rendering::framebuffer
