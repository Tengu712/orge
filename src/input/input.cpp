#include "input.hpp"

#include <unordered_map>
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

std::unordered_map<OrgeScancode, int32_t> g_states;

void update() {
	int numKeys = 0;
	const auto states = SDL_GetKeyboardState(&numKeys);
	for (const auto &n: ORGE_SDL_MAP) {
		if (numKeys <= static_cast<int>(n.second)) {
			continue;
		}
		const auto state = getState(n.first);
		// 押下
		if (states[n.second]) {
			// 押下中ならインクリメント
			if (state > 0) {
				g_states[n.first] = state + 1;
			}
			// そうでないなら1に設定
			else {
				g_states[n.first] = 1;
			}
		}
		// 非押下
		else {
			// 押下中なら-1に設定
			if (state > 0) {
				g_states[n.first] = -1;
			}
			// そうでないなら0に設定
			else {
				g_states[n.first] = 0;
			}
		}
	}
}

int32_t getState(OrgeScancode scancode) {
	if (input::g_states.contains(scancode)) {
		return input::g_states.at(scancode);
	} else {
		return 0;
	}
}

} // namespace input
