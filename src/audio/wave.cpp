#include "wave.hpp"

#include <charconv>
#include <format>
#define STB_VORBIS_IMPLEMENTATION
#include <stb_vorbis.c>
#include <string_view>

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

std::shared_ptr<Wave> createFromOggFile(const std::string &path, uint32_t startPosition) {
	using Vorbis = std::unique_ptr<stb_vorbis, decltype(&stb_vorbis_close)>;

	// ファイルオープン
	int error;
	const auto v = Vorbis(stb_vorbis_open_filename(path.c_str(), &error, nullptr), stb_vorbis_close);
	if (!v) {
		throw std::format("failed to open '{}': {}", path, error);
	}

	// 情報取得
	const auto info = stb_vorbis_get_info(v.get());
	if (info.channels <= 0 || info.sample_rate == 0) {
		throw std::format("the ogg file '{}' is invalid.", path);
	}
	const SDL_AudioSpec spec{SDL_AUDIO_F32, info.channels, static_cast<int>(info.sample_rate)};

	// サンプル数取得
	const auto frameCount = stb_vorbis_stream_length_in_samples(v.get());
	const auto sampleCount = frameCount * static_cast<unsigned int>(info.channels);

	// デコード
	auto data = new float[sampleCount];
	const auto decoded = stb_vorbis_get_samples_float_interleaved(v.get(), info.channels, data, sampleCount);
	if (decoded < 0 || static_cast<unsigned int>(decoded) != frameCount) {
		delete[] data;
		throw std::format("failed to decode '{}'.", path);
	}

	// バイナリデータとして解釈
	const auto length = static_cast<Uint32>(sampleCount * sizeof(float));
	auto buffer = reinterpret_cast<Uint8 *>(data);

	// ループ開始位置取得
	uint32_t startPositionFound = UINT32_MAX;
	const auto comment = stb_vorbis_get_comment(v.get());
	for (int i = 0; i < comment.comment_list_length; ++i) {
		const std::string_view ct(comment.comment_list[i]);
		if (!ct.starts_with("LOOPSTART=")) {
			continue;
		}
		const auto ctv = ct.substr(10);
		size_t lsv;
		const auto [p, e] = std::from_chars(ctv.data(), ctv.data() + ctv.size(), lsv);
		if (e == std::errc{}) {
			startPositionFound = static_cast<uint32_t>(lsv);
			break;
		}
	}
	if (startPositionFound == UINT32_MAX) {
		startPositionFound = startPosition;
	}

	// 終了
	return std::make_shared<Wave>(spec, buffer, length, startPositionFound);
}

std::shared_ptr<Wave> Wave::fromFile(const std::string &path, uint32_t startPosition) {
	if (path.ends_with(".wav") || path.ends_with(".wave") || path.ends_with(".WAV") || path.ends_with(".WAVE")) {
		return createFromWaveFile(path, startPosition);
	} else if (path.ends_with(".ogg") || path.ends_with(".OGG")) {
		return createFromOggFile(path, startPosition);
	} else {
		throw std::format("the format of the sound file '{}' is unsupported.", path);
	}
}

} // namespace audio
