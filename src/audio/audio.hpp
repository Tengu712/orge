#pragma once

#include "wave.hpp"

#include <unordered_map>
#include <vector>

namespace audio {

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

	float getVolume(uint32_t index) const;

	void setVolume(uint32_t index, float volume);

	void loadWaveFromFile(const std::string &file, uint32_t startPosition) {
		_waves.emplace(file, Wave::fromFile(file, startPosition));
	}

	void destroyWave(const std::string &file) noexcept {
		if (_waves.contains(file)) {
			_waves.erase(file);
		}
	}

	void play(const std::string &file, uint32_t index, bool loop);
};

void initialize();

void destroy() noexcept;

Audio &audio();

} // namespace audio
