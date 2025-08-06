#include "wave.hpp"

#include <format>

namespace audio {

std::shared_ptr<Wave> createFromWaveFile(const std::string &path, uint32_t startPosition) {
	SDL_AudioSpec spec;
	Uint8 *buffer;
	Uint32 length;
	if (!SDL_LoadWAV(path.c_str(), &spec, &buffer, &length)) {
		throw std::format("failed to load '{}'.", path);
	}
	return std::make_shared<Wave>(spec, buffer, length, startPosition);
}

std::shared_ptr<Wave> Wave::fromFile(const std::string &path, uint32_t startPosition) {
	if (path.ends_with(".wav") || path.ends_with(".wave") || path.ends_with(".WAV") || path.ends_with(".WAVE")) {
		return createFromWaveFile(path, startPosition);
	} else {
		throw std::format("the format of the sound file '{}' is unsupported.", path);
	}
}

} // namespace audio
