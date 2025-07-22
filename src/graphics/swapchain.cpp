#include "swapchain.hpp"

#include "platform.hpp"

#include <memory>
#include <SDL3/SDL_vulkan.h>

namespace graphics {

uint32_t getImageCount(const vk::SurfaceCapabilitiesKHR &caps) {
	if (caps.maxImageCount < 2) {
		throw "the surface not support double buffering.";
	}
	return caps.minImageCount > 2 ? caps.minImageCount : 2;
}

Window createWindow(const std::string &title, uint32_t width, uint32_t height, bool fullscreen) {
	const auto fsFlag = fullscreen ? SDL_WINDOW_FULLSCREEN : static_cast<SDL_WindowFlags>(0);
	const auto window = SDL_CreateWindow(title.c_str(), width, height, SDL_WINDOW_VULKAN | fsFlag);
	if (!window) {
		throw "failed to create a window.";
	}
	return Window(window, SDL_DestroyWindow);
}

vk::SurfaceKHR createSurface(const Window &window, const vk::Instance &instance) {
	VkSurfaceKHR surface;
	if (SDL_Vulkan_CreateSurface(window.get(), instance, NULL, &surface)) {
		return static_cast<vk::SurfaceKHR>(surface);
	} else {
		throw "failed to create a surface.";
	}
}

vk::SwapchainKHR createSwapchain(
	const vk::PhysicalDevice &physicalDevice,
	const vk::Device &device,
	const vk::SurfaceKHR &surface,
	const vk::Extent2D &extent
) {
	const auto caps = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	const auto ci = vk::SwapchainCreateInfoKHR()
		.setSurface(surface)
		.setMinImageCount(getImageCount(caps))
		.setImageFormat(platformRenderTargetPixelFormat())
		.setImageColorSpace(platformRenderTargetColorSpace())
		.setImageExtent(extent)
		.setImageArrayLayers(1)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
		.setImageSharingMode(vk::SharingMode::eExclusive)
		.setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
		.setPresentMode(vk::PresentModeKHR::eFifo)
		.setClipped(vk::True);
	return device.createSwapchainKHR(ci);
}

std::vector<vk::Image> getImagesFrom(
	const vk::PhysicalDevice &physicalDevice,
	const vk::Device &device,
	const vk::SurfaceKHR &surface,
	const vk::SwapchainKHR &swapchain
) {
	const auto caps = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	const auto imageCount = getImageCount(caps);
	auto images = device.getSwapchainImagesKHR(swapchain);
	if (images.size() < imageCount) {
		throw "swapchain images are too few.";
	}
	images.resize(imageCount);
	return images;
}

Swapchain::Swapchain(
	const std::string &title,
	uint32_t width,
	uint32_t height,
	bool fullscreen,
	const vk::Instance &instance,
	const vk::PhysicalDevice &physicalDevice,
	const vk::Device &device
) :
	_window(createWindow(title, width, height, fullscreen)),
	_surface(createSurface(_window, instance)),
	_extent(physicalDevice.getSurfaceCapabilitiesKHR(_surface).currentExtent),
	_swapchain(createSwapchain(physicalDevice, device, _surface, _extent)),
	_images(getImagesFrom(physicalDevice, device, _surface, _swapchain))
{
	// TODO: 決め打ちしない方が良いかもしれない。
	const auto formats = physicalDevice.getSurfaceFormatsKHR(_surface);
	const auto ok = std::any_of(formats.cbegin(), formats.cend(), [](const auto &n) {
		return n.format == platformRenderTargetPixelFormat() && n.colorSpace == platformRenderTargetColorSpace();
	});
	if (!ok) {
		throw "the surface color space is invalid.";
	}
}

} // namespace graphics
