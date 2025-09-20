#pragma once

#include "../graphics/graphics.hpp"
#include "error.hpp"

#include <SDL3/SDL.h>
#include <vulkan/vulkan.hpp>

#define ABORT_WITH_ERROR_DIALOG(msg) \
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "fatal error", (msg), nullptr); \
	abort()

namespace error {

inline void handleVkResult(const vk::Result &e) {
	switch (e) {
	case vk::Result::eErrorInitializationFailed:
		ABORT_WITH_ERROR_DIALOG("Failed to initialize Vulkan. Is Vulkan availabe and latest?");
	case vk::Result::eErrorLayerNotPresent:
		ABORT_WITH_ERROR_DIALOG("Some Vulkan layers required are unavailable. Is Vulkan availabe and latest?");
	case vk::Result::eErrorExtensionNotPresent:
		ABORT_WITH_ERROR_DIALOG("Some Vulkan extensions required are unavailable. Is Vulkan availabe and latest?");
	case vk::Result::eErrorDeviceLost:
		ABORT_WITH_ERROR_DIALOG("Graphics device has been lost.");
	case vk::Result::eErrorOutOfHostMemory:
		ABORT_WITH_ERROR_DIALOG("Host memory limit has been reached.");
	case vk::Result::eErrorOutOfDeviceMemory:
		ABORT_WITH_ERROR_DIALOG("Graphics device memory limit has been reached.");
	case vk::Result::eErrorInvalidVideoStdParametersKHR:
		ABORT_WITH_ERROR_DIALOG("Video Std parameter is invalid.");
	case vk::Result::eSuboptimalKHR:
	case vk::Result::eErrorOutOfDateKHR:
		setMessage("swapchain is out of date, performing recreation.");
		graphics::recreateSwapchain();
		break;
	case vk::Result::eErrorSurfaceLostKHR:
		error::setMessage("surface is out of date, performing recreation.");
		graphics::recreateSurface();
		break;
	default:
		error::setMessage("vulkan error: " + vk::to_string(e) + " (" + std::to_string(static_cast<int64_t>(e)) + ")");
		break;
	}
}

} // namespace error
