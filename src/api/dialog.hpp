#pragma once

#include <orge/orge.h>

#include "../error/error.hpp"

#include <SDL3/SDL.h>

namespace api {

inline void showDialog(OrgeShowDialogParam *param) {
	SDL_MessageBoxFlags flags = SDL_MESSAGEBOX_ERROR;
	switch (param->dtype) {
	case ORGE_DIALOG_TYPE_WARNING:
		flags = SDL_MESSAGEBOX_WARNING;
		break;
	case ORGE_DIALOG_TYPE_INFORMATION:
		flags = SDL_MESSAGEBOX_INFORMATION;
		break;
	default:
		break;
	}
	SDL_ShowSimpleMessageBox(flags, param->title, param->message, nullptr);
}

inline void showErrorDialog() {
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "error", error::getMessage().c_str(), nullptr);
}

} // namespace api
