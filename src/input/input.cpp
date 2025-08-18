#include "input.hpp"

#include "../config/config.hpp"

#include <SDL3/SDL.h>

namespace input {

const std::unordered_map<OrgeScancode, SDL_Scancode> ORGE_SDL_MAP{
	{ORGE_SCANCODE_A,            SDL_SCANCODE_A},
	{ORGE_SCANCODE_B,            SDL_SCANCODE_B},
	{ORGE_SCANCODE_C,            SDL_SCANCODE_C},
	{ORGE_SCANCODE_D,            SDL_SCANCODE_D},
	{ORGE_SCANCODE_E,            SDL_SCANCODE_E},
	{ORGE_SCANCODE_F,            SDL_SCANCODE_F},
	{ORGE_SCANCODE_G,            SDL_SCANCODE_G},
	{ORGE_SCANCODE_H,            SDL_SCANCODE_H},
	{ORGE_SCANCODE_I,            SDL_SCANCODE_I},
	{ORGE_SCANCODE_J,            SDL_SCANCODE_J},
	{ORGE_SCANCODE_K,            SDL_SCANCODE_K},
	{ORGE_SCANCODE_L,            SDL_SCANCODE_L},
	{ORGE_SCANCODE_M,            SDL_SCANCODE_M},
	{ORGE_SCANCODE_N,            SDL_SCANCODE_N},
	{ORGE_SCANCODE_O,            SDL_SCANCODE_O},
	{ORGE_SCANCODE_P,            SDL_SCANCODE_P},
	{ORGE_SCANCODE_Q,            SDL_SCANCODE_Q},
	{ORGE_SCANCODE_R,            SDL_SCANCODE_R},
	{ORGE_SCANCODE_S,            SDL_SCANCODE_S},
	{ORGE_SCANCODE_T,            SDL_SCANCODE_T},
	{ORGE_SCANCODE_U,            SDL_SCANCODE_U},
	{ORGE_SCANCODE_V,            SDL_SCANCODE_V},
	{ORGE_SCANCODE_W,            SDL_SCANCODE_W},
	{ORGE_SCANCODE_X,            SDL_SCANCODE_X},
	{ORGE_SCANCODE_Y,            SDL_SCANCODE_Y},
	{ORGE_SCANCODE_Z,            SDL_SCANCODE_Z},
	{ORGE_SCANCODE_1,            SDL_SCANCODE_1},
	{ORGE_SCANCODE_2,            SDL_SCANCODE_2},
	{ORGE_SCANCODE_3,            SDL_SCANCODE_3},
	{ORGE_SCANCODE_4,            SDL_SCANCODE_4},
	{ORGE_SCANCODE_5,            SDL_SCANCODE_5},
	{ORGE_SCANCODE_6,            SDL_SCANCODE_6},
	{ORGE_SCANCODE_7,            SDL_SCANCODE_7},
	{ORGE_SCANCODE_8,            SDL_SCANCODE_8},
	{ORGE_SCANCODE_9,            SDL_SCANCODE_9},
	{ORGE_SCANCODE_0,            SDL_SCANCODE_0},
	{ORGE_SCANCODE_RETURN,       SDL_SCANCODE_RETURN},
	{ORGE_SCANCODE_ESCAPE,       SDL_SCANCODE_ESCAPE},
	{ORGE_SCANCODE_BACKSPACE,    SDL_SCANCODE_BACKSPACE},
	{ORGE_SCANCODE_TAB,          SDL_SCANCODE_TAB},
	{ORGE_SCANCODE_SPACE,        SDL_SCANCODE_SPACE},
	{ORGE_SCANCODE_MINUS,        SDL_SCANCODE_MINUS},
	{ORGE_SCANCODE_EQUALS,       SDL_SCANCODE_EQUALS},
	{ORGE_SCANCODE_LEFTBRACKET,  SDL_SCANCODE_LEFTBRACKET},
	{ORGE_SCANCODE_RIGHTBRACKET, SDL_SCANCODE_RIGHTBRACKET},
	{ORGE_SCANCODE_BACKSLASH,    SDL_SCANCODE_BACKSLASH},
	{ORGE_SCANCODE_NONUSHASH,    SDL_SCANCODE_NONUSHASH},
	{ORGE_SCANCODE_SEMICOLON,    SDL_SCANCODE_SEMICOLON},
	{ORGE_SCANCODE_APOSTROPHE,   SDL_SCANCODE_APOSTROPHE},
	{ORGE_SCANCODE_GRAVE,        SDL_SCANCODE_GRAVE},
	{ORGE_SCANCODE_COMMA,        SDL_SCANCODE_COMMA},
	{ORGE_SCANCODE_PERIOD,       SDL_SCANCODE_PERIOD},
	{ORGE_SCANCODE_SLASH,        SDL_SCANCODE_SLASH},
	{ORGE_SCANCODE_RIGHT,        SDL_SCANCODE_RIGHT},
	{ORGE_SCANCODE_LEFT,         SDL_SCANCODE_LEFT},
	{ORGE_SCANCODE_DOWN,         SDL_SCANCODE_DOWN},
	{ORGE_SCANCODE_UP,           SDL_SCANCODE_UP},
};

const std::unordered_map<OrgeMouseButton, unsigned int> ORGE_SDL_MOUSE_MAP{
	{ORGE_MOUSE_BUTTON_LEFT,   SDL_BUTTON_LMASK},
	{ORGE_MOUSE_BUTTON_MIDDLE, SDL_BUTTON_MMASK},
	{ORGE_MOUSE_BUTTON_RIGHT,  SDL_BUTTON_RMASK},
};

int32_t calcNextState(bool down, int32_t prev) {
	// 押下
	if (down) {
		// 押下中ならインクリメント
		if (prev > 0) {
			return prev + 1;
		}
		// そうでないなら1に設定
		else {
			return 1;
		}
	}
	// 非押下
	else {
		// 押下中なら-1に設定
		if (prev > 0) {
			return -1;
		}
		// そうでないなら0に設定
		else {
			return 0;
		}
	}
}

void Input::update() {
	std::lock_guard lk(_mutex);

	int numKeys = 0;
	const auto states = SDL_GetKeyboardState(&numKeys);
	for (const auto &n: ORGE_SDL_MAP) {
		if (numKeys <= static_cast<int>(n.second)) {
			continue;
		}
		const auto state = _states.contains(n.first) ? _states[n.first] : 0;
		_states[n.first] = calcNextState(states[n.second], state);
	}

	float mouseX, mouseY;
	const auto mouseState = SDL_GetMouseState(&mouseX, &mouseY);
	const auto maxX = static_cast<float>(config::config().width - 1);
	const auto maxY = static_cast<float>(config::config().height - 1);
	if (mouseX >= 0.0f && mouseY >= 0.0f && mouseX <= maxX && mouseY <= maxY) {
		_cursorX = static_cast<uint32_t>(mouseX);
		_cursorY = static_cast<uint32_t>(mouseY);
	}

	for (const auto &n: ORGE_SDL_MOUSE_MAP) {
		const auto pressed = static_cast<bool>(mouseState & n.second);
		const auto state = _mouseButtonStates.contains(n.first) ? _mouseButtonStates[n.first] : 0;
		_mouseButtonStates[n.first] = calcNextState(pressed, state);
	}
}

} // namespace input
