#include "error.hpp"

const char *convertErrorMessage(Error from) {
	switch (from) {
		using enum Error;
		case None: return "";
		case CreateInstance: return "failed to create a Vulkan instance.";
		case CreateWindow: return "failed to create a window.";
		case SelectPhysicalDevice: return "failed to select a physical device.";
		case GetQueueFamilyIndex: return "failed to get a queue family index.";
		case CreateDevice: return "failed to create a Vulkan device.";
		case CreateCommandPool: return "failed to create a command pool.";
		case CreateSurface: return "failed to create a surface.";
		default: return "undefined error happened.";
	}
}
