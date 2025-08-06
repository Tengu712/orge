#pragma once

#include <memory>
#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <string>

namespace audio {

struct Wave {
	using Buffer = std::unique_ptr<Uint8, decltype(&SDL_free)>;

	const SDL_AudioSpec spec;
	const Buffer buffer;
	const Uint32 length;
	const uint32_t startPosition;

	Wave(const SDL_AudioSpec &spec, Uint8 *buffer, Uint32 length, uint32_t startPosition):
		spec(spec),
		buffer(Buffer(buffer, SDL_free)),
		length(length),
		startPosition(startPosition)
	{}

	static std::shared_ptr<Wave> fromFile(const std::string &path, uint32_t startPosition);
};

} // namespace audio
