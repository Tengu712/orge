#pragma once

#include <orge/orge.h>

#include "../audio/audio.hpp"

namespace api {

inline OrgeApiResult getAudioChannelVolume(OrgeGetAudioChannelVolumeParam *param) {
	try {
		*param->out = audio::audio().getVolume(param->index);
		return ORGE_OK;
	} catch (...) {
		return ORGE_AUDIO_CHANNEL_NOT_INITIALIZED;
	}
}

inline void setAudioChannelVolume(OrgeSetAudioChannelVolumeParam *param) {
	audio::audio().setVolume(param->index, param->volume);
}

inline void loadWave(OrgeLoadWaveParam *param) {
	audio::audio().loadWaveFromFile(param->file, param->startPosition);
}

inline void destroyWave(OrgeDestroyWaveParam *param) {
	audio::audio().destroyWave(param->file);
}

inline void playWave(OrgePlayWaveParam *param) {
	audio::audio().play(param->file, param->index, static_cast<bool>(param->loop));
}

} // namespace api
