#include "audio.hpp"

#include "../config/config.hpp"
#include "../error/error.hpp"

namespace audio {

bool isSameSpec(const SDL_AudioSpec &a, const SDL_AudioSpec &b) {
	return a.format == b.format && a.channels == b.channels && a.freq == b.freq;
}

Audio::Audio():
	_device(SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr)),
	_channels(config::config().audioChannelCount)
{
	if (_device == 0) {
		throw "failed to open an audio device.";
	}
}

void Audio::play(const std::string &id, uint32_t index) {
	if (index >= _channels.size()) {
		throw std::format("the number of audio channels is {} but tried to use {} th.", _channels.size(), index);
	}

	auto &wave = error::at(_waves, id, "waves");

	if (!_channels[index] || !isSameSpec(_channels[index]->spec, wave.spec)) {
		auto stream = SDL_CreateAudioStream(&wave.spec, nullptr);
		if (!stream) {
			throw "failed to create an audio stream.";
		}
		_channels[index] = std::make_unique<AudioChannel>(wave.spec, stream);
		if (!SDL_BindAudioStream(_device, _channels[index]->stream.get())) {
			throw "failed to bind an audio stream to the device.";
		}
	} else {
		SDL_ClearAudioStream(_channels[index]->stream.get());
	}

	SDL_PutAudioStreamData(_channels[index]->stream.get(), wave.buffer.get(), wave.length);
}

} // namespace audio
