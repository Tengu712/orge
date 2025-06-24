#include "swapchain.hpp"

#include <algorithm>
#include <vector>

namespace graphics::swapchain {

constexpr const char *EXTENSIONS[] = {"VK_KHR_swapchain"};
constexpr auto RENDER_TARGET_PIXEL_FORMAT = vk::Format::eR8G8B8A8Srgb;
constexpr auto RENDER_TARGET_COLOR_SPACE = vk::ColorSpaceKHR::eSrgbNonlinear;

vk::SurfaceKHR g_surface;
vk::SwapchainKHR g_swapchain;
uint32_t g_imageCount;
std::vector<vk::ImageView> g_imageViews;

std::span<const char *const> getExtensions() {
	return std::span(EXTENSIONS);
}

Error createSwapchain(const vk::PhysicalDevice &physicalDevice, const vk::Device &device) {
	try {
		// 色空間チェック
		const auto formats = physicalDevice.getSurfaceFormatsKHR(g_surface);
		const auto ok = std::any_of(formats.cbegin(), formats.cend(), [](const auto &n) {
			return n.format == RENDER_TARGET_PIXEL_FORMAT && n.colorSpace == RENDER_TARGET_COLOR_SPACE;
		});
		if (!ok) {
			return Error::InvalidColorSpace;
		}

		// バッファリングのイメージ数取得
		const auto caps = physicalDevice.getSurfaceCapabilitiesKHR(g_surface);
		g_imageCount = caps.minImageCount > 2 ? caps.minImageCount : 2;
		if (caps.maxImageCount < 2) {
			return Error::DoubleBufferingUnavailable;
		}

		// 作成
		const auto ci = vk::SwapchainCreateInfoKHR()
			.setSurface(g_surface)
			.setMinImageCount(g_imageCount)
			.setImageFormat(RENDER_TARGET_PIXEL_FORMAT)
			.setImageColorSpace(RENDER_TARGET_COLOR_SPACE)
			.setImageExtent(caps.currentExtent)
			.setImageArrayLayers(1)
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
			.setImageSharingMode(vk::SharingMode::eExclusive)
			.setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
			.setClipped(vk::True);
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
				.setFormat(RENDER_TARGET_PIXEL_FORMAT)
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

Error initialize(const vk::PhysicalDevice &physicalDevice, const vk::Device &device, const vk::SurfaceKHR &surface) {
	g_surface = surface;

	CHECK(createSwapchain(physicalDevice, device));
	CHECK(createImageViews(device));

	return Error::None;
}

void terminate(const vk::Device &device) {
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
