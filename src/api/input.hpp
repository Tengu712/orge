#pragma once

#include <orge/orge.h>

#include "../input/input.hpp"

namespace api {

inline void getKeyState(OrgeGetKeyStateParam *param) {
	*param->out = input::input().getState(param->scancode);
}

inline void getCursor(OrgeGetCursorParam *param) {
	*param->outX = input::input().getCursorX();
	*param->outY = input::input().getCursorY();
}

inline void getMouseButtonState(OrgeGetMouseButtonStateParam *param) {
	*param->out = input::input().getMouseButtonState(param->button);
}

} // namespace api
