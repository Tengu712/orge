#include "window.hpp"

#include <SDL3/SDL.h>

namespace graphics::window {

SDL_Window *g_window;

Error createWindow(const char *title, int width, int height) {
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		return Error::CreateWindow;
	}

	g_window = SDL_CreateWindow(title, width, height, 0);
	if (!g_window) {
		return Error::CreateWindow;
	}

	return Error::None;
}

void terminate() {
	if (g_window) {
		SDL_DestroyWindow(g_window);
		g_window = nullptr;
	}
}

} // namespace graphics::window
