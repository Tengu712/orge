#include <assetdef.hpp>
#include <fstream>
#include <format>
#include <iostream>
#include <unordered_set>
#include <vector>

std::vector<std::string> collectAssetNames(int argc, char *argv[]) {
	std::vector<std::string> result;
	std::unordered_set<std::string> checkSet;
	result.reserve(argc - 1);
	checkSet.reserve(argc - 1);
	for (int i = 1; i < argc; ++i) {
		if (!checkSet.contains(argv[i])) {
			result.emplace_back(argv[i]);
			checkSet.insert(argv[i]);
		}
	}
	return result;
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

void run(const std::vector<std::string> &fileNames) {
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
	const auto fileNames = collectAssetNames(argc, argv);
	if (fileNames.empty()) {
		std::cout << "usage: assetzip <asset1> <asset2> ..." << std::endl;
		std::cout << "nothing generated." << std::endl;
		return 0;
	}

	try {
		run(fileNames);
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		std::cerr << "failed to zip asset files." << std::endl;
		return 1;
	} catch (...) {
		std::cerr << "failed to zip asset files." << std::endl;
		return 1;
	}

	return 0;
}
