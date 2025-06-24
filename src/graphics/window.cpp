#include "window.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace graphics::window {

SDL_Window *g_window;

Error createWindow(const char *title, int width, int height) {
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		return Error::CreateWindow;
	}

	if (!SDL_Vulkan_LoadLibrary(nullptr)) {
		return Error::CreateWindow;
	}

	g_window = SDL_CreateWindow(title, width, height, SDL_WINDOW_VULKAN);
	if (!g_window) {
		return Error::CreateWindow;
	}

	return Error::None;
}

std::span<const char *const> getExtensions() {
	Uint32 count = 0;
	const char *const *extensions = SDL_Vulkan_GetInstanceExtensions(&count);
	if (!extensions || count == 0) {
		return {};
	}
	return std::span(extensions, count);
}

std::optional<vk::SurfaceKHR> createSurface(const vk::Instance &instance) {
	VkSurfaceKHR surface;
	if (SDL_Vulkan_CreateSurface(g_window, instance, NULL, &surface)) {
		return surface;
	} else {
		return std::nullopt;
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
