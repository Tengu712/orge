#include <orge.h>

#include "input/input.hpp"

int32_t orgeGetKeyState(uint32_t scancode) {
	return input::input().getState(static_cast<OrgeScancode>(scancode));
}

uint32_t orgeGetCursorX(void) {
	return input::input().getCursorX();
}

uint32_t orgeGetCursorY(void) {
	return input::input().getCursorY();
}

int32_t orgeGetMouseButtonState(uint32_t button) {
	return input::input().getMouseButtonState(static_cast<OrgeMouseButton>(button));
}
