#include <iostream>
#include <orge.h>

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

const std::string text = "Orgeのテキストレンダリングのサンプルです\n改行や!-_|のような記号も描画できます";

int main() {
	TRY(orgeInitializeWith("config.yml"));
	TRY(orgeRasterizeCharacters("font", "Text Rendering Sample"));
	TRY(orgeRasterizeCharacters("font", text.c_str()));

	float x = 0.0f;
	float y = 0.0f;
	OrgeTextLocationHorizontal horizontal = ORGE_TEXT_LOCATION_HORIZONTAL_LEFT;
	OrgeTextLocationVertical   vertical   = ORGE_TEXT_LOCATION_VERTICAL_TOP;
	while (orgeUpdate()) {
		if (orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_LEFT)) == 1) {
			x = 0.0f;
			horizontal = ORGE_TEXT_LOCATION_HORIZONTAL_LEFT;
		}
		if (orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_RIGHT)) == 1) {
			x = 640.0f;
			horizontal = ORGE_TEXT_LOCATION_HORIZONTAL_RIGHT;
		}
		if (orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_UP)) == 1) {
			y = 0.0f;
			vertical = ORGE_TEXT_LOCATION_VERTICAL_TOP;
		}
		if (orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_DOWN)) == 1) {
			y = 480.0f;
			vertical = ORGE_TEXT_LOCATION_VERTICAL_BOTTOM;
		}
		if (orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_RETURN)) == 1) {
			x = 320.0f;
			y = 240.0f;
			horizontal = ORGE_TEXT_LOCATION_HORIZONTAL_CENTER;
			vertical   = ORGE_TEXT_LOCATION_VERTICAL_MIDDLE;
		}

		CHECK(orgePutText(
			"PL",
			"font",
			"Text Rendering Sample",
			320.0f,
			120.0f,
			28.0f,
			static_cast<uint32_t>(ORGE_TEXT_LOCATION_HORIZONTAL_CENTER),
			static_cast<uint32_t>(ORGE_TEXT_LOCATION_VERTICAL_MIDDLE)
		));
		CHECK(orgePutText(
			"PL",
			"font",
			text.c_str(),
			x,
			y,
			28.0f,
			static_cast<uint32_t>(horizontal),
			static_cast<uint32_t>(vertical)
		));
		CHECK(orgeBeginRender());
		CHECK(orgeDrawTexts("PL"));
		CHECK(orgeEndRender());
	}

	orgeTerminate();
	return 0;
}
