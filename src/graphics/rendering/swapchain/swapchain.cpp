#include "swapchain.hpp"

#include "../../platform.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace graphics::rendering::swapchain {

std::string g_title;
uint32_t g_width = 0;
uint32_t g_height = 0;
SDL_WindowFlags g_fullscreenFlag = static_cast<SDL_WindowFlags>(0);

SDL_Window *g_window;
vk::SurfaceKHR g_surface;
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

	g_images.clear();

	if (g_window) {
		SDL_DestroyWindow(g_window);
		g_window = nullptr;
	}
}

std::span<const char *const> getInstanceExtensions() {
	Uint32 count = 0;
	const char *const *extensions = SDL_Vulkan_GetInstanceExtensions(&count);
	if (!extensions || count == 0) {
		return {};
	}
	return std::span(extensions, count);
}

std::span<const char *const> getDeviceExtensions() {
	static constexpr const char *EXTENSIONS[] = {"VK_KHR_swapchain"};
	return std::span(EXTENSIONS);
}

void create(const vk::Instance &instance, const vk::PhysicalDevice &physicalDevice, const vk::Device &device) {
	// ウィンドウ作成
	g_window = SDL_CreateWindow(g_title.c_str(), g_width, g_height, SDL_WINDOW_VULKAN | g_fullscreenFlag);
	if (!g_window) {
		throw "failed to create a window.";
	}

	// サーフェス作成
	VkSurfaceKHR surface;
	if (SDL_Vulkan_CreateSurface(g_window, instance, NULL, &surface)) {
		g_surface = static_cast<vk::SurfaceKHR>(surface);
	} else {
		throw "failed to create a surface.";
	}

	// フォーマットが正しいか確認
	const auto formats = physicalDevice.getSurfaceFormatsKHR(g_surface);
	const auto ok = std::any_of(formats.cbegin(), formats.cend(), [](const auto &n) {
		return n.format == platformRenderTargetPixelFormat() && n.colorSpace == platformRenderTargetColorSpace();
	});
	if (!ok) {
		throw "the surface color space is invalid.";
	}

	// capabilities取得
	const auto caps = physicalDevice.getSurfaceCapabilitiesKHR(g_surface);

	// イメージ数取得
	const auto imageCount = caps.minImageCount > 2 ? caps.minImageCount : 2;
	if (caps.maxImageCount < 2) {
		throw "the surface not support double buffering.";
	}

	// スワップチェイン作成
	const auto ci = vk::SwapchainCreateInfoKHR()
		.setSurface(g_surface)
		.setMinImageCount(imageCount)
		.setImageFormat(platformRenderTargetPixelFormat())
		.setImageColorSpace(platformRenderTargetColorSpace())
		.setImageExtent(caps.currentExtent)
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

void initialize(
	const config::Config &config,
	const vk::Instance &instance,
	const vk::PhysicalDevice &physicalDevice,
	const vk::Device &device
) {
	g_title = config.title;
	g_width = config.width;
	g_height = config.height;
	// TODO: configからフルスクリーンを指定できるように。
	g_fullscreenFlag = static_cast<SDL_WindowFlags>(0);
	create(instance, physicalDevice, device);
}

const std::vector<vk::Image> &getImages() {
	return g_images;
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

void toggleFullscreen(
	const vk::Instance &instance,
	const vk::PhysicalDevice &physicalDevice,
	const vk::Device &device
) {
	if (SDL_GetWindowFlags(g_window) & SDL_WINDOW_FULLSCREEN) {
		g_fullscreenFlag = static_cast<SDL_WindowFlags>(0);
	} else {
		g_fullscreenFlag = SDL_WINDOW_FULLSCREEN;
	}
	device.waitIdle();
	terminate(instance, device);
	create(instance, physicalDevice, device);
}

} // namespace graphics::rendering::swapchain
