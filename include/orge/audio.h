#pragma once

#include <stdint.h>

struct OrgeGetAudioChannelVolumeParam {
	uint32_t index;
	float *out;
};

struct OrgeSetAudioChannelVolumeParam {
	uint32_t index;
	float volume;
};

struct OrgeLoadWaveParam {
	const char *file;
	uint32_t startPosition;
};

struct OrgeDestroyWaveParam {
	const char *file;
};

struct OrgePlayWaveParam {
	const char *file;
	uint32_t index;
	uint8_t loop;
};
