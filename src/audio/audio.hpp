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
};

struct AudioChannel {
	using Stream = std::unique_ptr<SDL_AudioStream, decltype(&SDL_DestroyAudioStream)>;

	const Stream stream;
	std::shared_ptr<Wave> binding;
	bool loop;

	AudioChannel(SDL_AudioStream *stream):
		stream(Stream(stream, SDL_DestroyAudioStream)),
		binding(nullptr),
		loop(false)
	{}
};

class Audio {
private:
	const SDL_AudioDeviceID _device;
	std::vector<std::unique_ptr<AudioChannel>> _channels;
	std::unordered_map<std::string, std::shared_ptr<Wave>> _waves;

public:
	Audio(const Audio &) = delete;
	Audio(const Audio &&) = delete;
	Audio &operator =(const Audio &) = delete;
	Audio &operator =(const Audio &&) = delete;

	Audio();
	~Audio() {
		SDL_CloseAudioDevice(_device);
	}

	void update();

	void setVolume(uint32_t index, float volume);

	void loadWaveFromFile(const std::string &id, const std::string &path, uint32_t startPosition) {
		SDL_AudioSpec spec;
		Uint8 *buffer;
		Uint32 length;
		if (!SDL_LoadWAV(path.c_str(), &spec, &buffer, &length)) {
			throw "failed to load a WAV.";
		}
		_waves.emplace(id, std::make_shared<Wave>(spec, buffer, length, startPosition));
	}

	void destroyWave(const std::string &id) noexcept {
		if (_waves.contains(id)) {
			_waves.erase(id);
		}
	}

	void play(const std::string &id, uint32_t index, bool loop);
};

} // namespace audio
