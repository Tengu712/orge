#include "framebuffer.hpp"

#include "../config.hpp"
#include "platform.hpp"
#include "utils.hpp"

namespace graphics::framebuffer {

struct Attachment {
	const bool isRenderTarget;
	const vk::Image image;
	const vk::ImageView view;
	const vk::DeviceMemory memory;
};

std::vector<std::vector<Attachment>> g_attachments;
std::vector<vk::ClearValue> g_clearValues;
std::vector<vk::Framebuffer> g_framebuffers;

void terminate(const vk::Device &device) {
	for (auto &n: g_attachments) {
		for (auto &m: n) {
			if (!m.isRenderTarget) {
				device.freeMemory(m.memory);
			}
			device.destroyImageView(m.view);
			if (!m.isRenderTarget) {
				device.destroyImage(m.image);
			}
		}
	}
	g_attachments.clear();

	for (auto &n: g_framebuffers) {
		device.destroyFramebuffer(n);
	}
}

vk::Format convertFormat(const config::Format &format) {
	return format == config::Format::RenderTarget
		? platform::getRenderTargetPixelFormat()
		: format == config::Format::DepthBuffer
		? vk::Format::eD32Sfloat
		: throw;
}

vk::ImageUsageFlags convertUsage(const config::Format &format) {
	return format == config::Format::DepthBuffer
		? vk::ImageUsageFlagBits::eDepthStencilAttachment
		: throw;
}

vk::ImageAspectFlags convertAspect(const config::Format &format) {
	return format == config::Format::RenderTarget
		? vk::ImageAspectFlagBits::eColor
		: format == config::Format::DepthBuffer
		? vk::ImageAspectFlagBits::eDepth
		: throw;
}

Attachment createAttachment(
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::Extent3D &extent,
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
			.setExtent(extent)
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
		memory = utils::allocateImageMemory(
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
	return Attachment{isRenderTarget, image, view, memory};
}

void initialize(
	const config::Config &config,
	const vk::PhysicalDeviceMemoryProperties &memoryProps,
	const vk::Device &device,
	const vk::RenderPass &renderPass,
	const vk::Extent2D &extent,
	const std::vector<vk::Image> &swapchainImages
) {
	const auto extent3D = vk::Extent3D(extent.width, extent.height, 1);

	// アタッチメント作成 & クリアバリュー収集
	g_attachments.reserve(swapchainImages.size());
	for (const auto &n: config.attachments) {
		// アタッチメント作成
		std::vector<Attachment> attachments;
		attachments.reserve(swapchainImages.size());
		for (size_t i = 0; i < swapchainImages.size(); ++i) {
			attachments.push_back(createAttachment(memoryProps, device, extent3D, swapchainImages[i], n.format));
		}
		g_attachments.push_back(std::move(attachments));

		// クリアバリュー収集
		if (n.colorClearValue) {
			g_clearValues.emplace_back(static_cast<vk::ClearValue>(vk::ClearColorValue(n.colorClearValue.value())));
		} else {
			g_clearValues.emplace_back(static_cast<vk::ClearValue>(vk::ClearDepthStencilValue(n.depthClearValue.value(), 0)));
		}
	}

	// フレームバッファ作成
	g_framebuffers.reserve(swapchainImages.size());
	for (size_t i = 0; i < swapchainImages.size(); ++i) {
		// アタッチメント収集
		std::vector<vk::ImageView> attachments;
		attachments.reserve(g_attachments.size());
		for (const auto &n: g_attachments) {
			attachments.push_back(n[i].view);
		}

		// 作成
		const auto ci = vk::FramebufferCreateInfo()
			.setRenderPass(renderPass)
			.setAttachments(attachments)
			.setWidth(extent.width)
			.setHeight(extent.height)
			.setLayers(1);
		g_framebuffers.push_back(device.createFramebuffer(ci));
	}
}

const std::vector<vk::ClearValue> &getClearValues() {
	return g_clearValues;
}

const vk::Framebuffer &getFramebuffer(uint32_t index) {
	return g_framebuffers.at(index);
}

} // namespace graphics::framebuffer
