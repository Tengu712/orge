#pragma once

#include <mutex>
#include <orge.h>
#include <unordered_map>

namespace input {

class Input {
private:
	mutable std::mutex _mutex;
	std::unordered_map<OrgeScancode, int32_t> _states;
	uint32_t _cursorX = 0;
	uint32_t _cursorY = 0;
	std::unordered_map<OrgeMouseButton, int32_t> _mouseButtonStates;

public:
	void update();

	uint32_t getCursorX() const noexcept {
		std::lock_guard lk(_mutex);
		return _cursorX;
	}

	uint32_t getCursorY() const noexcept {
		std::lock_guard lk(_mutex);
		return _cursorY;
	}

	int32_t getState(OrgeScancode scancode) const noexcept {
		std::lock_guard lk(_mutex);
		if (_states.contains(scancode)) {
			return _states.at(scancode);
		} else {
			return 0;
		}
	}

	int32_t getMouseButtonState(OrgeMouseButton button) const noexcept {
		std::lock_guard lk(_mutex);
		if (_mouseButtonStates.contains(button)) {
			return _mouseButtonStates.at(button);
		} else {
			return 0;
		}
	}
};

} // namespace input
