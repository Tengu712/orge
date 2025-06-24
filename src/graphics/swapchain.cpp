#include "swapchain.hpp"

#include <algorithm>

namespace graphics::swapchain {

constexpr const char *EXTENSIONS[] = {"VK_KHR_swapchain"};
constexpr auto RENDER_TARGET_PIXEL_FORMAT = vk::Format::eR8G8B8A8Srgb;
constexpr auto RENDER_TARGET_COLOR_SPACE = vk::ColorSpaceKHR::eSrgbNonlinear;

vk::SurfaceKHR g_surface;
vk::SwapchainKHR g_swapchain;

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

		// バッファリングチェック
		const auto caps = physicalDevice.getSurfaceCapabilitiesKHR(g_surface);
		const auto minImageCount = caps.minImageCount > 2 ? caps.minImageCount : 2;
		if (caps.maxImageCount < 2) {
			return Error::DoubleBufferingUnavailable;
		}

		// 作成
		const auto ci = vk::SwapchainCreateInfoKHR()
			.setSurface(g_surface)
			.setMinImageCount(minImageCount)
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

Error initialize(const vk::PhysicalDevice &physicalDevice, const vk::Device &device, const vk::SurfaceKHR &surface) {
	g_surface = surface;

	CHECK(createSwapchain(physicalDevice, device));

	return Error::None;
}

void terminate() {
	if (g_surface) {
		g_surface = nullptr;
	}
}

} // namespace graphics::swapchain
