#include "error.hpp"

const char *convertErrorMessage(Error from) {
	switch (from) {
		using enum Error;
		case None: return "";
		case CreateInstance: return "failed to create a Vulkan instance.";
		case CreateWindow: return "failed to create a window.";
		case SelectPhysicalDevice: return "failed to select physical device.";
		case CreateDevice: return "failed to create a Vulkan device.";
		default: return "undefined error happened.";
	}
}
