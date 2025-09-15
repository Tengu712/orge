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

void Audio::update() {
	for (const auto &n: _channels) {
		if (!n || !n->loop || !n->binding) {
			continue;
		}
		const auto length = n->binding->length - n->binding->startByte;
		if (SDL_GetAudioStreamQueued(n->stream.get()) >= static_cast<int>(length)) {
			continue;
		}
		const auto buffer = n->binding->buffer.get() + n->binding->startByte;
		if (!SDL_PutAudioStreamData(n->stream.get(), static_cast<const void *>(buffer), static_cast<int>(length))) {
			throw "failed to put a wave data to the stream.";
		}
	}
}

float Audio::getVolume(uint32_t index) const {
	auto &channel = error::at(_channels, index, "channels");
	if (!channel || !channel->stream) {
		throw std::format("the {} th audio channel is uninitialized.", index);
	}
	return SDL_GetAudioStreamGain(channel->stream.get());
}

void Audio::setVolume(uint32_t index, float volume) {
	if (volume < 0.0f || volume > 1.0f) {
		throw std::format("the audio channel volume must be between 0 and 1 but passed {}.", volume);
	}
	auto &channel = error::at(_channels, index, "channels");
	if (!channel || !channel->stream) {
		throw std::format("the {} th audio channel is uninitialized.", index);
	}
	if (!SDL_SetAudioStreamGain(channel->stream.get(), volume)) {
		throw "failed to set the volume of an audio stream.";
	}
}

void Audio::play(const std::string &file, uint32_t index, bool loop) {
	auto &wave = error::at(_waves, file, "waves");
	auto &channel = error::at(_channels, index, "channels");

	if (!channel || !channel->binding || !isSameSpec(channel->binding->spec, wave->spec)) {
		auto stream = SDL_CreateAudioStream(&wave->spec, nullptr);
		if (!stream) {
			throw "failed to create an audio stream.";
		}
		_channels[index] = std::make_unique<AudioChannel>(stream);
		if (!SDL_BindAudioStream(_device, _channels[index]->stream.get())) {
			_channels[index] = nullptr;
			throw "failed to bind an audio stream to the device.";
		}
	} else if (!SDL_ClearAudioStream(channel->stream.get())) {
		throw "failed to clear an audio stream.";
	}

	channel->binding = wave;
	channel->loop = loop;

	if (!SDL_PutAudioStreamData(channel->stream.get(), wave->buffer.get(), wave->length)) {
		throw "failed to put a wave data to the stream.";
	}
}

std::optional<Audio> g_audio;

void initialize() {
	if (g_audio) {
		throw "audio already initialized.";
	}
	g_audio.emplace();
}

void destroy() noexcept {
	g_audio.reset();
}

Audio &audio() {
	if (g_audio) {
		return g_audio.value();
	} else {
		throw "audio not initialized.";
	}
}

} // namespace audio
