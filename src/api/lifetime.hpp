#pragma once

#include <orge/orge.h>

#include "../asset/asset.hpp"
#include "../config/config.hpp"
#include "../graphics/graphics.hpp"
#include "../graphics/core/core.hpp"
#include "../graphics/window/swapchain.hpp"
#include "../audio/audio.hpp"
#include "../input/input.hpp"

#include <format>
#include <SDL3/SDL_vulkan.h>

#ifdef __APPLE__
# define MODKEY SDL_KMOD_GUI
#else
# define MODKEY SDL_KMOD_ALT
#endif

namespace api {

inline void initialize() {
	if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
		throw std::format("failed to prepare for creating a window: {}", SDL_GetError());
	}
	if (!SDL_Vulkan_LoadLibrary(nullptr)) {
		throw std::format("failed to load Vulkan: {}", SDL_GetError());
	}
	asset::initialize();
	// TODO: param化
	config::initialize();
	graphics::initialize();
	audio::initialize();
	input::initialize();
}

inline void terminate() {
	graphics::terminate();
	audio::destroy();
	input::destroy();
}

inline OrgeApiResult update() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_EVENT_QUIT) {
			return ORGE_WINDOW_CLOSED;
		}
		if (
			config::config().altReturnToggleFullscreen
				&& event.type == SDL_EVENT_KEY_DOWN
				&& event.key.key == SDLK_RETURN
				&& (event.key.mod & MODKEY)
		) {
			const auto &swapchain = graphics::window::swapchain();
			graphics::core::device().waitIdle();
			swapchain.setFullscreen(!swapchain.isFullscreen());
		}
	}
	audio::audio().update();
	input::input().update();
	return ORGE_OK;
}

} // namespace api
