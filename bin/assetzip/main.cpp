#include <fstream>
#include <format>
#include <iostream>
#include <unordered_map>
#include <vector>

struct AssetHeader {
	const uint32_t count;
};

struct AssetEntry {
	const uint32_t id;
	const uint32_t offset;
	const uint32_t size;

	AssetEntry(uint32_t id, uint32_t offset, uint32_t size): id(id), offset(offset), size(size) {}
};

std::vector<unsigned char> loadFile(const std::string &path) {
	std::ifstream file(path, std::ios::binary);
	if (!file) {
		throw std::runtime_error(std::format("failed to load '{}'.", path));
	}

	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<unsigned char> data(size);
	file.read(reinterpret_cast<char*>(data.data()), size);

	return data;
}

void run(const std::vector<std::string> &paths) {
	// TODO: 総アセットファイルサイズが大きすぎるとやばいのでどうにかする。
	// アセットファイルをすべて読み込み
	std::unordered_map<uint32_t, std::vector<unsigned char>> assets;
	for (const auto &n: paths) {
		// TODO: config.ymlからIDを読み取る。
		assets.emplace(0, loadFile(n));
	}

	// ヘッダー構築
	const AssetHeader header{static_cast<uint32_t>(assets.size())};

	// エントリー構築
	auto offset = sizeof(AssetHeader) + sizeof(AssetEntry) * assets.size();
	std::vector<AssetEntry> entries;
	for (const auto &[id, data]: assets) {
		entries.emplace_back(id, static_cast<uint32_t>(offset), static_cast<uint32_t>(data.size()));
		offset += static_cast<uint32_t>(data.size());
	}

	std::ofstream out(".dat", std::ios::binary);
	if (!out) {
		throw std::runtime_error("failed to create '.dat'.");
	}

	out.write(reinterpret_cast<const char *>(&header), sizeof(AssetHeader));
	out.write(reinterpret_cast<const char *>(entries.data()), sizeof(AssetEntry) * entries.size());
	for (const auto &n: entries) {
		const auto &data = assets[n.id];
		out.write(reinterpret_cast<const char *>(data.data()), data.size());
	}
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "no asset file passed." << std::endl;
		return 1;
	}

	try {
		run(std::vector<std::string>(argv + 1, argv + argc));
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		std::cerr << "failed to zip asset files." << std::endl;
	}

	return 0;
}
