#include <iostream>
#include <orge.h>
#include <vector>

#define TRY(n) \
	if (!(n)) { \
		std::cout << orgeGetErrorMessage() << std::endl; \
		return 1; \
	}

#define CHECK(n) \
	if (!(n)) { \
		std::cout << orgeGetErrorMessage() << std::endl; \
		continue; \
	}

const std::vector<float> RED  {1.0f, 0.0f, 0.0f, 1.0f};
const std::vector<float> GREEN{0.0f, 1.0f, 0.0f, 1.0f};
const std::vector<float> BLUE {0.0f, 0.0f, 1.0f, 1.0f};

int main() {
	TRY(orgeInitialize());

	TRY(orgeCreateBuffer("red",   static_cast<uint64_t>(sizeof(float) * RED.size()),   0, 0));
	TRY(orgeCreateBuffer("green", static_cast<uint64_t>(sizeof(float) * GREEN.size()), 0, 0));
	TRY(orgeCreateBuffer("blue",  static_cast<uint64_t>(sizeof(float) * BLUE.size()),  0, 0));
	TRY(orgeUpdateBuffer("red",   reinterpret_cast<const uint8_t *>(RED.data())));
	TRY(orgeUpdateBuffer("green", reinterpret_cast<const uint8_t *>(GREEN.data())));
	TRY(orgeUpdateBuffer("blue",  reinterpret_cast<const uint8_t *>(BLUE.data())));

	uint32_t state = 0;
	uint32_t cursorX = 0;
	uint32_t cursorY = 0;
	while (orgeUpdate()) {
		const auto keyState = orgeGetKeyState(ORGE_SCANCODE_RETURN);
		if (keyState == 1) {
			std::cout << "return key pressed" << std::endl;
			state = (state + 1) % 3;
		}
		if (keyState > 0) {
			std::cout << "return key down" << std::endl;
		}
		if (keyState == -1) {
			std::cout << "return key released" << std::endl;
		}

		const auto newCursorX = orgeGetCursorX();
		const auto newCursorY = orgeGetCursorY();
		if (cursorX != newCursorX || cursorY != newCursorY) {
			std::cout << "cursor: (" << newCursorX << ", " << newCursorY << ")" << std::endl;
			cursorX = newCursorX;
			cursorY = newCursorY;
		}

		const auto mouseState = orgeGetMouseButtonState(static_cast<uint32_t>(ORGE_MOUSE_BUTTON_LEFT));
		if (mouseState == 1) {
			std::cout << "left mouse button pressed" << std::endl;
		}
		if (mouseState > 0) {
			std::cout << "left mouse button down" << std::endl;
		}
		if (mouseState == -1) {
			std::cout << "left mouse button released" << std::endl;
		}

		CHECK(orgeUpdateBufferDescriptor("RP", "PL", "red",   0, 0, 0, 0));
		CHECK(orgeUpdateBufferDescriptor("RP", "PL", "green", 0, 1, 0, 0));
		CHECK(orgeUpdateBufferDescriptor("RP", "PL", "blue",  0, 2, 0, 0));

		const std::vector<uint32_t> sets{state};

		CHECK(orgeBeginRender());
		CHECK(orgeBeginRenderPass("RP"));
		CHECK(orgeBindPipeline("PL", sets.data()));
		CHECK(orgeDrawDirectly(3, 1, 0));
		CHECK(orgeEndRenderPass());
		CHECK(orgeEndRender());
	}

	orgeTerminate();
	return 0;
}
