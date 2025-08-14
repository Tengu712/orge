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

#define LOG(n) \
	if (!(n)) { \
		std::cout << orgeGetErrorMessage() << std::endl; \
	}

int main() {
	TRY(orgeInitialize());
	TRY(orgeLoadWave("cdefgabc.wav", 58100));
	TRY(orgeLoadWave("music.ogg", 0));

	while (orgeUpdate()) {
		if (orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_RETURN)) == 1) {
			LOG(orgeSetAudioChannelVolume(0, 0.25f));
			std::cout << "current volume: " << orgeGetAudioChannelVolume(0) << std::endl;
		}
		if (orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_SPACE)) == 1) {
			LOG(orgeSetAudioChannelVolume(0, 0.75f));
			std::cout << "current volume: " << orgeGetAudioChannelVolume(0) << std::endl;
		}

		if (orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_1)) == 1) {
			LOG(orgePlayWave("cdefgabc.wav", 0, 0));
		}
		if (orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_2)) == 1) {
			LOG(orgePlayWave("cdefgabc.wav", 1, 0));
		}

		if (orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_Q)) == 1) {
			LOG(orgePlayWave("cdefgabc.wav", 0, 1));
		}
		if (orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_W)) == 1) {
			LOG(orgePlayWave("music.ogg", 0, 1));
		}

		CHECK(orgeBeginRender());
		CHECK(orgeEndRender());
	}

	orgeTerminate();
	return 0;
}
