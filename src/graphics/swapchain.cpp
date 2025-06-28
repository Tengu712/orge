#include "swapchain.hpp"

#include "platform.hpp"

#include <algorithm>
#include <vector>

namespace graphics::swapchain {

constexpr const char *EXTENSIONS[] = {"VK_KHR_swapchain"};

vk::SurfaceKHR g_surface;
vk::SwapchainKHR g_swapchain;
uint32_t g_imageCount;
vk::Extent2D g_imageSize;
std::vector<vk::ImageView> g_imageViews;
vk::Semaphore g_waitForImageEnabledSemaphore;
vk::Semaphore g_waitForRenderingSemaphore;

std::span<const char *const> getDeviceExtensions() {
	return std::span(EXTENSIONS);
}

Error validateColorSpace(const vk::PhysicalDevice &physicalDevice) {
	try {
		const auto formats = physicalDevice.getSurfaceFormatsKHR(g_surface);
		const auto ok = std::any_of(formats.cbegin(), formats.cend(), [](const auto &n) {
			return n.format == platform::getRenderTargetPixelFormat() && n.colorSpace == platform::getRenderTargetColorSpace();
		});
		return ok ? Error::None : Error::InvalidColorSpace;
	} catch (...) {
		return Error::InvalidColorSpace;
	}
}

Error getImageCountAndSize(const vk::PhysicalDevice &physicalDevice) {
	try {
		const auto caps = physicalDevice.getSurfaceCapabilitiesKHR(g_surface);
		g_imageCount = caps.minImageCount > 2 ? caps.minImageCount : 2;
		g_imageSize = caps.currentExtent;
		return caps.maxImageCount >= 2 ? Error::None : Error::DoubleBufferingUnavailable;
	} catch (...) {
		return Error::DoubleBufferingUnavailable;
	}
}

Error createSwapchain(const vk::PhysicalDevice &physicalDevice, const vk::Device &device) {
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
		.setClipped(vk::True);
	try {
		g_swapchain = device.createSwapchainKHR(ci);
	} catch (...) {
		return Error::CreateSwapchain;
	}
	return Error::None;
}

Error createImageViews(const vk::Device &device) {
	try {
		const auto images = device.getSwapchainImagesKHR(g_swapchain);
		if (images.size() < g_imageCount) {
			return Error::CreateSwapchainImageView;
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
	} catch (...) {
		return Error::CreateSwapchainImageView;
	}
	return Error::None;
}

Error createSemaphores(const vk::Device &device) {
	const auto ci = vk::SemaphoreCreateInfo();
	try {
		g_waitForImageEnabledSemaphore = device.createSemaphore(ci);
		g_waitForRenderingSemaphore    = device.createSemaphore(ci);
	} catch (...) {
		return Error::CreateSemaphoresForSwapchain;
	}
	return Error::None;
}

Error initialize(const vk::PhysicalDevice &physicalDevice, const vk::Device &device, const vk::SurfaceKHR &surface) {
	g_surface = surface;

	CHECK(validateColorSpace(physicalDevice));
	CHECK(getImageCountAndSize(physicalDevice));
	CHECK(createSwapchain(physicalDevice, device));
	CHECK(createImageViews(device));
	CHECK(createSemaphores(device));

	return Error::None;
}

std::vector<vk::Framebuffer> createFrameBuffers(const vk::Device &device, const vk::RenderPass &renderPass) {
	std::vector<vk::Framebuffer> framebuffers;
	framebuffers.reserve(g_imageCount);
	for (const auto &n: g_imageViews) {
		const auto ci = vk::FramebufferCreateInfo()
			.setRenderPass(renderPass)
			.setAttachments({n})
			.setWidth(g_imageSize.width)
			.setHeight(g_imageSize.height)
			.setLayers(1);
		try {
			framebuffers.push_back(device.createFramebuffer(ci));
		} catch (...) {
			framebuffers.clear();
			break;
		}
	}
	return framebuffers;
}

void terminate(const vk::Device &device) {
	if (g_waitForRenderingSemaphore) {
		device.destroySemaphore(g_waitForRenderingSemaphore);
		g_waitForRenderingSemaphore = nullptr;
	}
	if (g_waitForImageEnabledSemaphore) {
		device.destroySemaphore(g_waitForImageEnabledSemaphore);
		g_waitForImageEnabledSemaphore = nullptr;
	}
	if (!g_imageViews.empty()) {
		for (auto& imageView : g_imageViews) {
			device.destroyImageView(imageView);
		}
		g_imageViews.clear();
	}
	if (g_swapchain) {
		device.destroySwapchainKHR(g_swapchain);
		g_swapchain = nullptr;
	}
	if (g_surface) {
		g_surface = nullptr;
	}
}

} // namespace graphics::swapchain
