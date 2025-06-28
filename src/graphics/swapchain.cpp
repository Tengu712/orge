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
// ダミーフェンス
// acquireNextImageKHRを待機する必要はないもののセマフォもフェンスも与えないと怒られるので
vk::Fence g_dummyFenceForImageAvailable;

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
		.setPresentMode(vk::PresentModeKHR::eFifo)
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

Error createDummyFence(const vk::Device &device) {
	try {
		g_dummyFenceForImageAvailable = device.createFence({});
	} catch (...) {
		return Error::CreateDummyFenceForAcquireNextImageIndex;
	}
	return Error::None;
}

Error initialize(const vk::PhysicalDevice &physicalDevice, const vk::Device &device, const vk::SurfaceKHR &surface) {
	g_surface = surface;
	CHECK(validateColorSpace(physicalDevice));
	CHECK(getImageCountAndSize(physicalDevice));
	CHECK(createSwapchain(physicalDevice, device));
	CHECK(createImageViews(device));
	CHECK(createDummyFence(device));
	return Error::None;
}

Error createFramebuffers(const vk::Device &device, const vk::RenderPass &renderPass, std::vector<vk::Framebuffer> &framebuffers) {
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
			return Error::CreateFramebuffer;
		}
	}
	return Error::None;
}

Error acquireNextImageIndex(const vk::Device &device, uint32_t &index) {
	try {
		index = device.acquireNextImageKHR(g_swapchain, UINT64_MAX, nullptr, g_dummyFenceForImageAvailable).value;
		return Error::None;
	} catch (...) {
		return Error::AcquireNextImageIndex;
	}
}

vk::Extent2D getImageSize() {
	return g_imageSize;
}

Error presentation(const vk::Queue &queue, const vk::Semaphore &semaphore, uint32_t index) {
	const auto pi = vk::PresentInfoKHR()
		.setWaitSemaphores({semaphore})
		.setSwapchains({g_swapchain})
		.setImageIndices({index});
	try {
		const auto result = queue.presentKHR(pi);
		return result == vk::Result::eSuccess ? Error::None : Error::Presentation;
	} catch (...) {
		return Error::Presentation;
	}
}

void terminate(const vk::Device &device) {
	if (g_dummyFenceForImageAvailable) {
		device.destroyFence(g_dummyFenceForImageAvailable);
		g_dummyFenceForImageAvailable = nullptr;
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
