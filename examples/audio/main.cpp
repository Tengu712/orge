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
	TRY(orgeLoadWaveFromFile("cdefgabc", "cdefgabc.wav", 58100));

	while (orgeUpdate()) {
		if (orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_1)) == 1) {
			CHECK(orgePlayWave("cdefgabc", 0, 0));
		}
		if (orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_2)) == 1) {
			CHECK(orgePlayWave("cdefgabc", 1, 0));
		}

		if (orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_Q)) == 1) {
			CHECK(orgePlayWave("cdefgabc", 0, 1));
		}

		CHECK(orgeBeginRender());
		CHECK(orgeEndRender());
	}

	orgeTerminate();
	return 0;
}
