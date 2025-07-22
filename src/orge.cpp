#include <orge.h>

#include "config/config.hpp"
#include "error/error.hpp"
#include "graphics/graphics.hpp"
#include "input/input.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#ifdef __APPLE__
# define MODKEY SDL_KMOD_GUI
#else
# define MODKEY SDL_KMOD_ALT
#endif

namespace {

std::optional<config::Config> g_config;

int initialize() {
	TRY(
		if (!SDL_Init(SDL_INIT_VIDEO)) {
			throw "failed to prepare for creating a window.";
		}
		if (!SDL_Vulkan_LoadLibrary(nullptr)) {
			throw "failed to load Vulkan.";
		}
		graphics::initialize(g_config.value());
	)
}

} // namespace

int orgeInitialize(const char *yaml) {
	TRY(
		g_config = config::parse(yaml);
		initialize();
	)
}

int orgeInitializeWith(const char *yamlFilePath) {
	TRY(
		g_config = config::parseFromFile(yamlFilePath);
		initialize();
	)
}

void orgeTerminate(void) {
	graphics::terminate();
}

int orgeUpdate(void) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_EVENT_QUIT) {
			return 0;
		}
		if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_RETURN && (event.key.mod & MODKEY)) {
			// TODO: エラーハンドリング。
			graphics::toggleFullscreen(g_config.value());
		}
	}
	input::update();
	return 1;
}
