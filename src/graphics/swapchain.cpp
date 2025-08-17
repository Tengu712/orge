#include "swapchain.hpp"

#include "../config/config.hpp"

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
	if (!SDL_SetWindowFullscreenMode(window, nullptr)) {
		throw "failed to set borderless fullscreen desktop mode.";
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

vk::Format getFormatFrom(const vk::PhysicalDevice &physicalDevice, const vk::SurfaceKHR &surface) {
	const std::vector<vk::Format> favorites{
		vk::Format::eB8G8R8A8Srgb,
		vk::Format::eR8G8B8A8Srgb,
		vk::Format::eB8G8R8A8Unorm,
		vk::Format::eR8G8B8A8Unorm,
	};
	const auto formats = physicalDevice.getSurfaceFormatsKHR(surface);
	for (const auto &n: favorites) {
		const auto ok = std::any_of(formats.cbegin(), formats.cend(), [&n](const auto &m) {
			return m.format == n && m.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
		});
		if (ok) {
			return n;
		}
	}
	throw "the surface format and color space are invalid.";
}

vk::SwapchainKHR createSwapchain(
	const vk::PhysicalDevice &physicalDevice,
	const vk::Device &device,
	const vk::SurfaceKHR &surface,
	const vk::Extent2D &extent,
	const vk::Format &format,
	const vk::SwapchainKHR &oldSwapchain = nullptr
) {
	const auto caps = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	const auto ci = vk::SwapchainCreateInfoKHR()
		.setSurface(surface)
		.setMinImageCount(getImageCount(caps))
		.setImageFormat(format)
		.setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
		.setImageExtent(extent)
		.setImageArrayLayers(1)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
		.setImageSharingMode(vk::SharingMode::eExclusive)
		.setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
		.setPresentMode(config::config().disableVsync ? vk::PresentModeKHR::eImmediate : vk::PresentModeKHR::eFifo)
		.setClipped(vk::True)
		.setOldSwapchain(oldSwapchain);
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
	_width(width),
	_height(height),
	_window(createWindow(title, width, height, fullscreen)),
	_surface(createSurface(_window, instance)),
	_extent(physicalDevice.getSurfaceCapabilitiesKHR(_surface).currentExtent),
	_format(getFormatFrom(physicalDevice, _surface)),
	_swapchain(createSwapchain(physicalDevice, device, _surface, _extent, _format)),
	_images(getImagesFrom(physicalDevice, device, _surface, _swapchain))
{}

void Swapchain::recreateSwapchain(const vk::PhysicalDevice &physicalDevice, const vk::Device &device) {
	const auto old = _swapchain;
	_extent = physicalDevice.getSurfaceCapabilitiesKHR(_surface).currentExtent;
	_format = getFormatFrom(physicalDevice, _surface);
	_swapchain = createSwapchain(physicalDevice, device, _surface, _extent, _format, old);
	_images = getImagesFrom(physicalDevice, device, _surface, _swapchain);
	device.destroySwapchainKHR(old);
}

void Swapchain::recreateSurface(
	const vk::Instance &instance,
	const vk::PhysicalDevice &physicalDevice,
	const vk::Device &device
) {
	const auto old = _surface;
	_surface = createSurface(_window, instance);
	recreateSwapchain(physicalDevice, device);
	instance.destroySurfaceKHR(old);
}

} // namespace graphics
