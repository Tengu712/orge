#include <orge.h>

#include "audio/audio.hpp"
#include "orge-private.hpp"

float orgeGetAudioChannelVolume(uint32_t index) {
	try {
		return audio::audio().getVolume(index);
	} catch (...) {
		return -1.0f;
	}
}

uint8_t orgeSetAudioChannelVolume(uint32_t index, float volume) {
	TRY(audio::audio().setVolume(index, volume));
}

uint8_t orgeLoadWave(const char *file, uint32_t startPosition) {
	TRY(audio::audio().loadWaveFromFile(file, startPosition));
}

void orgeDestroyWave(const char *file) {
	audio::audio().destroyWave(file);
}

uint8_t orgePlayWave(const char *file, uint32_t index, uint8_t loop) {
	TRY(audio::audio().play(file, index, static_cast<bool>(loop)));
}
