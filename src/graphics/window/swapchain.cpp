#include "swapchain.hpp"

#include "../../config/config.hpp"
#include "../core/core.hpp"

#include <format>
#include <SDL3/SDL_vulkan.h>

namespace graphics::window {

uint32_t getImageCount(const vk::SurfaceCapabilitiesKHR &caps) {
	if (caps.maxImageCount < 2) {
		throw "the surface not support double buffering.";
	}
	return caps.minImageCount > 2 ? caps.minImageCount : 2;
}

Window createWindow() {
	const auto title = config::config().title;
	const auto width = config::config().width;
	const auto height = config::config().height;
	const auto fullscreen = config::config().fullscreen;

	const auto fsFlag = fullscreen ? SDL_WINDOW_FULLSCREEN : static_cast<SDL_WindowFlags>(0);
	const auto window = SDL_CreateWindow(title.c_str(), width, height, SDL_WINDOW_VULKAN | fsFlag);
	if (!window) {
		throw std::format("failed to create a window: {}", SDL_GetError());
	}
	if (!SDL_SetWindowFullscreenMode(window, nullptr)) {
		throw std::format("failed to set borderless fullscreen desktop mode: {}", SDL_GetError());
	}
	return Window(window, SDL_DestroyWindow);
}

vk::SurfaceKHR createSurface(const Window &window) {
	VkSurfaceKHR surface;
	if (SDL_Vulkan_CreateSurface(window.get(), core::instance(), NULL, &surface)) {
		return static_cast<vk::SurfaceKHR>(surface);
	} else {
		throw std::format("failed to create a surface: {}", SDL_GetError());
	}
}

vk::Format getFormatFrom(const vk::SurfaceKHR &surface) {
	const std::vector<vk::Format> favorites{
		vk::Format::eB8G8R8A8Srgb,
		vk::Format::eR8G8B8A8Srgb,
		vk::Format::eB8G8R8A8Unorm,
		vk::Format::eR8G8B8A8Unorm,
	};
	const auto formats = core::physicalDevice().getSurfaceFormatsKHR(surface);
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
	const vk::SurfaceKHR &surface,
	const vk::Extent2D &extent,
	const vk::Format &format,
	const vk::SwapchainKHR &oldSwapchain = nullptr
) {
	const auto caps = core::physicalDevice().getSurfaceCapabilitiesKHR(surface);
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
	return core::device().createSwapchainKHR(ci);
}

std::vector<vk::Image> getImagesFrom(const vk::SurfaceKHR &surface, const vk::SwapchainKHR &swapchain) {
	const auto caps = core::physicalDevice().getSurfaceCapabilitiesKHR(surface);
	const auto imageCount = getImageCount(caps);
	auto images = core::device().getSwapchainImagesKHR(swapchain);
	if (images.size() < imageCount) {
		throw "swapchain images are too few.";
	}
	images.resize(imageCount);
	return images;
}

Swapchain::Swapchain():
	_width(config::config().width),
	_height(config::config().height),
	_window(createWindow()),
	_surface(createSurface(_window)),
	_extent(core::physicalDevice().getSurfaceCapabilitiesKHR(_surface).currentExtent),
	_format(getFormatFrom(_surface)),
	_swapchain(createSwapchain(_surface, _extent, _format)),
	_images(getImagesFrom(_surface, _swapchain))
{}

Swapchain::~Swapchain() {
	core::device().destroySwapchainKHR(_swapchain);
	core::instance().destroySurfaceKHR(_surface);
}

void Swapchain::recreateSwapchain() {
	const auto old = _swapchain;
	_extent = core::physicalDevice().getSurfaceCapabilitiesKHR(_surface).currentExtent;
	_format = getFormatFrom(_surface);
	_swapchain = createSwapchain(_surface, _extent, _format, old);
	_images = getImagesFrom(_surface, _swapchain);
	core::device().destroySwapchainKHR(old);
}

void Swapchain::recreateSurface() {
	const auto old = _surface;
	_surface = createSurface(_window);
	recreateSwapchain();
	core::instance().destroySurfaceKHR(old);
}

uint32_t Swapchain::acquireNextImageIndex(const vk::Semaphore &semaphore) {
	const auto result = core::device().acquireNextImageKHR(_swapchain, UINT64_MAX, semaphore, nullptr);
	if (result.result == vk::Result::eSuccess) {
		return result.value;
	} else {
		throw result.result;
	}
}

void Swapchain::present(const vk::Semaphore &semaphore, uint32_t index) const {
	const auto pi = vk::PresentInfoKHR()
		.setWaitSemaphores({semaphore})
		.setSwapchains({_swapchain})
		.setImageIndices({index});
	if (core::queue().presentKHR(pi) != vk::Result::eSuccess) {
		throw "failed to present the screen.";
	}
}

std::optional<Swapchain> g_swapchain;

void initializeSwapchain() {
	if (g_swapchain) {
		throw "swapchain already initialized.";
	}
	g_swapchain.emplace();
}

void destroySwapchain() noexcept {
	g_swapchain = std::nullopt;
}

Swapchain &swapchain() {
	if (g_swapchain) {
		return g_swapchain.value();
	} else {
		throw "swapchain not initialized.";
	}
}

} // namespace graphics::window
