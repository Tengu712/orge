#include "swapchain.hpp"

#include "platform.hpp"
#include "window.hpp"

namespace graphics::swapchain {

constexpr const char *EXTENSIONS[] = {"VK_KHR_swapchain"};

vk::SurfaceKHR g_surface;
vk::Extent2D g_imageSize;
vk::SwapchainKHR g_swapchain;
std::vector<vk::Image> g_images;

void terminate(const vk::Instance &instance, const vk::Device &device) {
	if (g_swapchain) {
		device.destroySwapchainKHR(g_swapchain);
		g_swapchain = nullptr;
	}

	if (g_surface) {
		instance.destroySurfaceKHR(g_surface);
		g_surface = nullptr;
	}
}

std::span<const char *const> getDeviceExtensions() {
	return std::span(EXTENSIONS);
}

void initialize(const vk::Instance &instance, const vk::PhysicalDevice &physicalDevice, const vk::Device &device) {
	// サーフェス作成
	g_surface = window::createSurface(instance);

	// フォーマットが正しいか確認
	const auto formats = physicalDevice.getSurfaceFormatsKHR(g_surface);
	const auto ok = std::any_of(formats.cbegin(), formats.cend(), [](const auto &n) {
		return n.format == platform::getRenderTargetPixelFormat() && n.colorSpace == platform::getRenderTargetColorSpace();
	});
	if (!ok) {
		throw "the surface color space is invalid.";
	}

	// イメージサイズ取得
	const auto caps = physicalDevice.getSurfaceCapabilitiesKHR(g_surface);
	g_imageSize = caps.currentExtent;

	// イメージ数取得
	const auto imageCount = caps.minImageCount > 2 ? caps.minImageCount : 2;
	if (caps.maxImageCount < 2) {
		throw "the surface not support double buffering.";
	}

	// スワップチェイン作成
	const auto ci = vk::SwapchainCreateInfoKHR()
		.setSurface(g_surface)
		.setMinImageCount(imageCount)
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

	// イメージ数分イメージ取得
	g_images = device.getSwapchainImagesKHR(g_swapchain);
	if (g_images.size() < imageCount) {
		throw "swapchain images are too few.";
	}
	g_images.resize(imageCount);
}

const std::vector<vk::Image> &getImages() {
	return g_images;
}

vk::Extent2D getImageSize() {
	return g_imageSize;
}

uint32_t acquireNextImageIndex(const vk::Device &device, const vk::Semaphore &semaphore) {
	// TODO: これ例外投げられる？
	return device.acquireNextImageKHR(g_swapchain, UINT64_MAX, semaphore, nullptr).value;
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

} // namespace graphics::swapchain
