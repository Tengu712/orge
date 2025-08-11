#include <assetdef.hpp>
#include <fstream>
#include <format>
#include <iostream>
#include <set>
#include <vector>
#include <yaml-cpp/yaml.h>

std::vector<std::string> parseAssetFileNames(const std::string &yamlFilePath) {
	const auto node = YAML::LoadFile(yamlFilePath);

	std::vector<std::string> paths;
	paths.push_back(yamlFilePath);

	if (!node["assets"]) {
		return paths;
	}
	if (!node["assets"].IsSequence()) {
		throw std::runtime_error("YAML must contain 'assets' as a sequence.");
	}

	for (const auto &n: node["assets"]) {
		paths.push_back(n.as<std::string>());
	}

	std::set<std::string> pathSet;
	for (const auto &n: paths) {
		if (pathSet.contains(n)) {
			throw std::runtime_error(std::format("'{}' duplicated.", n));
		}
		pathSet.emplace(n);
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

void run(const std::string &yamlFilePath) {
	const auto fileNames = parseAssetFileNames(yamlFilePath);
	if (fileNames.empty()) {
		throw std::runtime_error("no asset files specified in config.");
	}

	// .dat出力開始
	std::ofstream out(".dat", std::ios::binary);
	if (!out) {
		throw std::runtime_error("failed to create '.dat'.");
	}

	// ヘッダー書込み
	const AssetHeader header{static_cast<uint32_t>(fileNames.size())};
	out.write(reinterpret_cast<const char *>(&header), sizeof(AssetHeader));

	// エントリーは後回し
	const auto entriesPos = out.tellp();
	std::vector<char> placeholder(sizeof(AssetEntry) * fileNames.size(), 0);
	out.write(placeholder.data(), placeholder.size());

	// データ書込み & エントリー構築
	std::vector<AssetEntry> entries;
	for (size_t i = 0; i < fileNames.size(); ++i) {
		const auto data = loadFile(fileNames[i]);
		entries.emplace_back(
			static_cast<uint32_t>(i),
			static_cast<uint32_t>(out.tellp()),
			static_cast<uint32_t>(data.size())
		);
		out.write(reinterpret_cast<const char *>(data.data()), data.size());
	}

	// エントリー書込み
	out.seekp(entriesPos);
	out.write(reinterpret_cast<const char *>(entries.data()), sizeof(AssetEntry) * entries.size());

	std::cout << "successfully zipped " << fileNames.size() - 1 << " assets." << std::endl;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << "usage: assetzip <config-file-path>" << std::endl;
		return 1;
	}

	try {
		run(argv[1]);
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
