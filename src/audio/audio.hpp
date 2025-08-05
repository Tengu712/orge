#pragma once

#include <memory>
#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace audio {

struct Wave {
	using Buffer = std::unique_ptr<Uint8, decltype(&SDL_free)>;

	SDL_AudioSpec spec;
	Buffer buffer;
	Uint32 length;

	Wave(const SDL_AudioSpec &spec, Uint8 *buffer, Uint32 length):
		spec(spec),
		buffer(Buffer(buffer, SDL_free)),
		length(length)
	{}
};

struct AudioChannel {
	using Stream = std::unique_ptr<SDL_AudioStream, decltype(&SDL_DestroyAudioStream)>;

	const SDL_AudioSpec spec;
	const Stream stream;

	AudioChannel(const SDL_AudioSpec &spec, SDL_AudioStream *stream):
		spec(spec),
		stream(Stream(stream, SDL_DestroyAudioStream))
	{}
};

class Audio {
private:
	const SDL_AudioDeviceID _device;
	std::vector<std::unique_ptr<AudioChannel>> _channels;
	std::unordered_map<std::string, Wave> _waves;

public:
	Audio(const Audio &) = delete;
	Audio(const Audio &&) = delete;
	Audio &operator =(const Audio &) = delete;
	Audio &operator =(const Audio &&) = delete;

	Audio();
	~Audio() {
		SDL_CloseAudioDevice(_device);
	}

	void loadWaveFromFile(const std::string &id, const std::string &path) {
		SDL_AudioSpec spec;
		Uint8 *buffer;
		Uint32 length;
		if (!SDL_LoadWAV(path.c_str(), &spec, &buffer, &length)) {
			throw "failed to load a WAV.";
		}
		_waves.emplace(id, Wave(spec, buffer, length));
	}

	void destroyWave(const std::string &id) noexcept {
		if (_waves.contains(id)) {
			_waves.erase(id);
		}
	}

	void play(const std::string &id, uint32_t index);
};

} // namespace audio
