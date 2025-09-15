#include <orge.h>

#include "asset/asset.hpp"
#include "audio/audio.hpp"
#include "config/config.hpp"
#include "error/error.hpp"
#include "graphics/core/core.hpp"
#include "graphics/graphics.hpp"
#include "graphics/window/swapchain.hpp"
#include "input/input.hpp"
#include "orge-private.hpp"

#include <cstdlib>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#ifdef __APPLE__
# define MODKEY SDL_KMOD_GUI
#else
# define MODKEY SDL_KMOD_ALT
#endif

// ================================================================================================================== //
//     Information                                                                                                    //
// ================================================================================================================== //

void orgeShowDialog(uint32_t dtype, const char *title, const char *message) {
	SDL_MessageBoxFlags flags = SDL_MESSAGEBOX_ERROR;
	switch (static_cast<OrgeDialogType>(dtype)) {
	case ORGE_DIALOG_TYPE_WARNING:
		flags = SDL_MESSAGEBOX_WARNING;
		break;
	case ORGE_DIALOG_TYPE_INFORMATION:
		flags = SDL_MESSAGEBOX_INFORMATION;
		break;
	default:
		break;
	}
	SDL_ShowSimpleMessageBox(flags, title, message, nullptr);
}

const char *orgeGetErrorMessage(void) {
	return error::getMessage().c_str();
}

// ================================================================================================================== //
//     Lifetime Managiment                                                                                            //
// ================================================================================================================== //

uint8_t orgeInitialize(void) {
	TRY(
		if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
			throw std::format("failed to prepare for creating a window: {}", SDL_GetError());
		}
		if (!SDL_Vulkan_LoadLibrary(nullptr)) {
			throw std::format("failed to load Vulkan: {}", SDL_GetError());
		}
		asset::initialize();
		config::initialize();
		graphics::initialize();
		audio::initialize();
		input::initialize();
	)
}

void orgeTerminate(void) {
	graphics::terminate();
	audio::destroy();
	input::destroy();
}

uint8_t orgeUpdate(void) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_EVENT_QUIT) {
			return 0;
		}
		if (
			config::config().altReturnToggleFullscreen
				&& event.type == SDL_EVENT_KEY_DOWN
				&& event.key.key == SDLK_RETURN
				&& (event.key.mod & MODKEY)
		) {
			// TODO: 例外どうしよう。
			const auto &swapchain = graphics::window::swapchain();
			graphics::core::device().waitIdle();
			swapchain.setFullscreen(!swapchain.isFullscreen());
		}
	}
	// TODO: 例外どうしよう。
	audio::audio().update();
	input::input().update();
	return 1;
}
