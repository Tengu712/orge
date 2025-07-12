#include "swapchain.hpp"

#include "platform.hpp"
#include "window.hpp"

#include <algorithm>
#include <vector>

namespace graphics::swapchain {

constexpr const char *EXTENSIONS[] = {"VK_KHR_swapchain"};

vk::SurfaceKHR g_surface;
vk::SwapchainKHR g_swapchain;
uint32_t g_imageCount;
vk::Extent2D g_imageSize;
std::vector<vk::ImageView> g_imageViews;

std::span<const char *const> getDeviceExtensions() {
	return std::span(EXTENSIONS);
}

void validateColorSpace(const vk::PhysicalDevice &physicalDevice) {
	const auto formats = physicalDevice.getSurfaceFormatsKHR(g_surface);
	const auto ok = std::any_of(formats.cbegin(), formats.cend(), [](const auto &n) {
		return n.format == platform::getRenderTargetPixelFormat() && n.colorSpace == platform::getRenderTargetColorSpace();
	});
	if (!ok) {
		throw "the surface color space is invalid.";
	}
}

void getImageCountAndSize(const vk::PhysicalDevice &physicalDevice) {
	const auto caps = physicalDevice.getSurfaceCapabilitiesKHR(g_surface);
	g_imageCount = caps.minImageCount > 2 ? caps.minImageCount : 2;
	g_imageSize = caps.currentExtent;
	if (caps.maxImageCount < 2) {
		throw "the surface not support double buffering.";
	}
}

void createSwapchain(const vk::Device &device) {
	const auto ci = vk::SwapchainCreateInfoKHR()
		.setSurface(g_surface)
		.setMinImageCount(g_imageCount)
		.setImageFormat(platform::getRenderTargetPixelFormat())
		.setImageColorSpace(platform::getRenderTargetColorSpace())
		.setImageExtent(g_imageSize)
		.setImageArrayLayers(1)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
		.setImageSharingMode(vk::SharingMode::eExclusive)
		.setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
		.setPresentMode(vk::PresentModeKHR::eFifo)
		.setClipped(vk::True);
	g_swapchain = device.createSwapchainKHR(ci);
}

void createImageViews(const vk::Device &device) {
	const auto images = device.getSwapchainImagesKHR(g_swapchain);
	if (images.size() < static_cast<size_t>(g_imageCount)) {
		throw "the number of swapchain image is too few.";
	}

	g_imageViews.reserve(g_imageCount);
	for (uint32_t i = 0; i < g_imageCount; ++i) {
		const auto ci = vk::ImageViewCreateInfo()
			.setImage(images[i])
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(platform::getRenderTargetPixelFormat())
			.setSubresourceRange(
				vk::ImageSubresourceRange()
					.setAspectMask(vk::ImageAspectFlagBits::eColor)
					.setLevelCount(1)
					.setLayerCount(1)
			);
		g_imageViews.push_back(device.createImageView(ci));
	}
}

void initialize(const vk::Instance &instance, const vk::PhysicalDevice &physicalDevice, const vk::Device &device) {
	g_surface = window::createSurface(instance);
	validateColorSpace(physicalDevice);
	getImageCountAndSize(physicalDevice);
	createSwapchain(device);
	createImageViews(device);
}

std::vector<vk::Framebuffer> createFramebuffers(const vk::Device &device, const vk::RenderPass &renderPass) {
	std::vector<vk::Framebuffer> framebuffers;
	framebuffers.reserve(g_imageCount);
	for (const auto &n: g_imageViews) {
		const auto ci = vk::FramebufferCreateInfo()
			.setRenderPass(renderPass)
			.setAttachments({n})
			.setWidth(g_imageSize.width)
			.setHeight(g_imageSize.height)
			.setLayers(1);
		framebuffers.push_back(device.createFramebuffer(ci));
	}
	return framebuffers;
}

uint32_t acquireNextImageIndex(const vk::Device &device, const vk::Semaphore &semaphore) {
	// TODO: これ例外投げられる？
	return device.acquireNextImageKHR(g_swapchain, UINT64_MAX, semaphore, nullptr).value;
}

uint32_t getImageCount() {
	return g_imageCount;
}

vk::Extent2D getImageSize() {
	return g_imageSize;
}

void presentation(const vk::Queue &queue, const vk::Semaphore &semaphore, uint32_t index) {
	const auto pi = vk::PresentInfoKHR()
		.setWaitSemaphores({semaphore})
		.setSwapchains({g_swapchain})
		.setImageIndices({index});
	if (queue.presentKHR(pi) != vk::Result::eSuccess) {
		throw "failed to present the screen.";
	}
}

void terminate(const vk::Instance &instance, const vk::Device &device) {
	for (auto& imageView : g_imageViews) {
		device.destroyImageView(imageView);
	}
	g_imageViews.clear();

	if (g_swapchain) {
		device.destroySwapchainKHR(g_swapchain);
		g_swapchain = nullptr;
	}

	if (g_surface) {
		instance.destroySurfaceKHR(g_surface);
		g_surface = nullptr;
	}
}

} // namespace graphics::swapchain
