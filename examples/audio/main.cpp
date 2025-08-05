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

int main() {
	TRY(orgeInitializeWith("config.yml"));
	TRY(orgeLoadWaveFromFile("cdefgabc", "cdefgabc.wav"));

	while (orgeUpdate()) {
		if (orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_1)) == 1) {
			CHECK(orgePlayWave("cdefgabc", 0));
		}

		CHECK(orgeBeginRender());
		CHECK(orgeEndRender());
	}

	orgeTerminate();
	return 0;
}
