#include <SDL3/SDL.h>
#include <vulkan/vulkan.hpp>

namespace {
	vk::Instance g_instance;
}

bool orgeInitialize() {
	try {
		vk::InstanceCreateInfo ci;
		g_instance = vk::createInstance(ci);
	} catch (...) {
		return false;
	}

	if (!SDL_Init(SDL_INIT_VIDEO)) {
		return false;
	}

	return true;
}

void orgeTerminate() {
	SDL_Quit();
	g_instance.destroy();
}
