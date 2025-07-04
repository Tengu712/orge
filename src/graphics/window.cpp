#include "window.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace graphics::window {

SDL_Window *g_window;

void initialize(const std::string &title, int width, int height) {
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		throw "failed to prepare for creating a window.";
	}
	if (!SDL_Vulkan_LoadLibrary(nullptr)) {
		throw "failed to load Vulkan.";
	}
	g_window = SDL_CreateWindow(title.c_str(), width, height, SDL_WINDOW_VULKAN);
	if (!g_window) {
		throw "failed to create a window.";
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

vk::SurfaceKHR createSurface(const vk::Instance &instance) {
	VkSurfaceKHR surface;
	if (SDL_Vulkan_CreateSurface(g_window, instance, NULL, &surface)) {
		return surface;
	} else {
		throw "failed to create a surface.";
	}
}

bool pollEvents() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_EVENT_QUIT) {
			return false;
		}
	}
	return true;
}

void terminate() {
	if (g_window) {
		SDL_DestroyWindow(g_window);
		g_window = nullptr;
	}
}

} // namespace graphics::window
