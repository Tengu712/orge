#include "asset.hpp"

#include <assetdef.hpp>
#include <fstream>
#include <unordered_map>
#include <vector>

namespace asset {

std::vector<unsigned char> g_dat;
std::unordered_map<uint32_t, AssetEntry> g_assetMap;

void loadDatFile() {
	// TODO: ファイル名を指定できるように
	std::ifstream file(".dat", std::ios::binary);
	if (!file) {
		throw ".dat not found.";
	}

	file.seekg(0, std::ios::end);
	const auto size = file.tellg();
	file.seekg(0, std::ios::beg);

	g_dat.clear();
	g_dat.resize(size);
	file.read(reinterpret_cast<char *>(g_dat.data()), size);
}

void analyzeDat() {
	size_t offset = 0;

	// ヘッダー取得
	if (g_dat.size() < sizeof(AssetHeader)) {
		throw ".dat is invalid: no header.";
	}
	const AssetHeader *header = reinterpret_cast<const AssetHeader *>(g_dat.data());
	offset += sizeof(AssetHeader);

	// エントリの先頭取得
	if (g_dat.size() < offset + sizeof(AssetEntry) * header->count) {
		throw ".dat is invalid: too small.";
	}
	const AssetEntry *entries = reinterpret_cast<const AssetEntry *>(g_dat.data() + offset);

	// エントリ取得
	for (uint32_t i = 0; i < header->count; ++i) {
		const auto &entry = entries[i];
		if (entry.offset + entry.size > g_dat.size()) {
			throw ".dat is invalid: too small.";
		}
		g_assetMap[entry.id] = entry;
	}
}

void initialize() {
	loadDatFile();
	analyzeDat();
}

std::span<const unsigned char> getAsset(uint32_t id) {
	const auto &entry = g_assetMap[id];
	const unsigned char *data = g_dat.data() + entry.offset;
	return std::span<const unsigned char>(data, entry.size);
}

} // namespace asset
