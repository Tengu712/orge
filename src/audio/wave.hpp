#pragma once

#include <memory>
#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <string>

namespace audio {

inline uint32_t calcLoopStartByte(const SDL_AudioSpec &spec, uint32_t startPosition) {
	switch (spec.format) {
	case SDL_AUDIO_U8:
	case SDL_AUDIO_S8:
		return startPosition * spec.channels;
	case SDL_AUDIO_S16LE:
	case SDL_AUDIO_S16BE:
		return startPosition * spec.channels * 2;
	case SDL_AUDIO_S32LE:
	case SDL_AUDIO_S32BE:
	case SDL_AUDIO_F32LE:
	case SDL_AUDIO_F32BE:
		return startPosition * spec.channels * 4;
	default:
		return startPosition * spec.channels;
	}
}

struct Wave {
	using Buffer = std::unique_ptr<Uint8, decltype(&SDL_free)>;

	const SDL_AudioSpec spec;
	const Buffer buffer;
	const Uint32 length;
	const uint32_t startByte;

	Wave(const SDL_AudioSpec &spec, Uint8 *buffer, Uint32 length, uint32_t startPosition):
		spec(spec),
		buffer(Buffer(buffer, SDL_free)),
		length(length),
		startByte(calcLoopStartByte(spec, startPosition))
	{}

	static std::shared_ptr<Wave> fromFile(const std::string &path, uint32_t startPosition);
};

} // namespace audio
