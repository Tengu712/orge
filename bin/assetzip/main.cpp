#include <assetdef.hpp>
#include <fstream>
#include <format>
#include <iostream>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <optional>
#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <yaml-cpp/yaml.h>

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

std::vector<unsigned char> encryptData(const std::vector<unsigned char> &data, const std::string &passphrase) {
	using CipherContext = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;

	std::vector<unsigned char> key(KEY_SIZE);
	std::vector<unsigned char> iv(IV_SIZE);

	EVP_BytesToKey(
		EVP_aes_256_cbc(),
		EVP_sha256(),
		nullptr,
		reinterpret_cast<const unsigned char *>(passphrase.c_str()),
		passphrase.length(),
		1,
		key.data(),
		iv.data()
	);

	auto ctx = CipherContext(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
	EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_cbc(), nullptr, key.data(), iv.data());

	std::vector<unsigned char> encrypted(data.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));

	int len, lenadd;
	EVP_EncryptUpdate(ctx.get(), encrypted.data(), &len, data.data(), data.size());
	EVP_EncryptFinal_ex(ctx.get(), encrypted.data() + len, &lenadd);

	encrypted.resize(len + lenadd);

	std::vector<unsigned char> result;
	result.insert(result.end(), iv.begin(), iv.end());
	result.insert(result.end(), encrypted.begin(), encrypted.end());

	return result;
}

void run(const std::string &passphrase, const std::optional<std::string> &yamlFile) {
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

	// 生データ作成
	std::vector<unsigned char> raw;
	raw.insert(
		raw.end(),
		reinterpret_cast<const unsigned char *>(&header),
		reinterpret_cast<const unsigned char *>(&header) + sizeof(AssetHeader)
	);
	raw.insert(
		raw.end(),
		reinterpret_cast<const unsigned char *>(entries.data()),
		reinterpret_cast<const unsigned char *>(entries.data()) + sizeof(AssetEntry) * entries.size()
	);
	for (const auto &n: entries) {
		const auto &data = assets[n.id];
		raw.insert(raw.end(), data.begin(), data.end());
	}

	// 暗号化
	auto encrypted = encryptData(raw, passphrase);

	// .dat出力
	std::ofstream out(".dat", std::ios::binary);
	if (!out) {
		throw std::runtime_error("failed to create '.dat'.");
	}
	out.write(reinterpret_cast<const char *>(encrypted.data()), encrypted.size());

	std::cout << "successfully zipped " << assets.size() << " assets." << std::endl;
}

int main(int argc, char* argv[]) {
	if (argc < 2 || argc > 3) {
		std::cerr << "usage: assetzip <passphrase> [<config-yaml-file-path>]" << std::endl;
		return 1;
	}

	try {
		run(argv[1], (argc == 3) ? std::make_optional(argv[2]) : std::nullopt);
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
