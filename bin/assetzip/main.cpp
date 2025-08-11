#include <fstream>
#include <format>
#include <iostream>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <yaml-cpp/yaml.h>

struct AssetHeader {
	const uint32_t count;
};

struct AssetEntry {
	const uint32_t id;
	const uint32_t offset;
	const uint32_t size;

	AssetEntry(uint32_t id, uint32_t offset, uint32_t size): id(id), offset(offset), size(size) {}
};

YAML::Node parseYaml(const std::optional<std::string> &yamlFile) {
	if (yamlFile) {
		return YAML::LoadFile(yamlFile.value());
	} else {
		std::ostringstream buffer;
		buffer << std::cin.rdbuf();
		return YAML::Load(buffer.str());
	}
}

std::vector<std::string> parseAssetFileNames(const YAML::Node &node) {
	if (!node["assets"] || !node["assets"].IsSequence()) {
		throw std::runtime_error("YAML must contain 'assets' as a sequence.");
	}

	std::vector<std::string> paths;
	for (const auto &n: node["assets"]) {
		paths.push_back(n.as<std::string>());
	}
	return paths;
}

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

void run(const std::optional<std::string> &yamlFile) {
	const auto fileNames = parseAssetFileNames(parseYaml(yamlFile));
	if (fileNames.empty()) {
		throw std::runtime_error("no asset files specified in config.");
	}

	// TODO: 総アセットファイルサイズが大きすぎるとやばいのでどうにかする。
	// アセットファイルをすべて読み込み
	std::unordered_map<uint32_t, std::vector<unsigned char>> assets;
	for (size_t i = 0; i < fileNames.size(); ++i) {
		assets.emplace(static_cast<uint32_t>(i), loadFile(fileNames[i]));
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

	std::cout << "successfully zipped " << assets.size() << " assets." << std::endl;
}

int main(int argc, char* argv[]) {
	if (argc > 2) {
		std::cerr << "usage: assetzip [<config-yaml-file-path>]" << std::endl;
		return 1;
	}

	try {
		run((argc == 2) ? std::make_optional(argv[1]) : std::nullopt);
	} catch (const YAML::Exception &e) {
		std::cerr << e.what() << std::endl;
		std::cerr << "failed to zip asset files." << std::endl;
		return 1;
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		std::cerr << "failed to zip asset files." << std::endl;
		return 1;
	}

	return 0;
}
