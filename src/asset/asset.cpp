#include "asset.hpp"

#include <fstream>
#include <memory>
#include <openssl/evp.h>
#include <unordered_map>
#include <vector>

namespace asset {

struct AssetHeader {
	const uint32_t count;
};

struct AssetEntry {
	uint32_t id;
	uint32_t offset;
	uint32_t size;
};

constexpr size_t KEY_SIZE = 32;
constexpr size_t IV_SIZE = 16;

std::vector<unsigned char> g_dat;
std::unordered_map<uint32_t, AssetEntry> g_assetMap;

std::vector<unsigned char> loadDatFile() {
	std::ifstream file(".dat", std::ios::binary);
	if (!file) {
		throw ".dat not found.";
	}

	file.seekg(0, std::ios::end);
	const auto size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<unsigned char> buffer(size);
	file.read(reinterpret_cast<char *>(buffer.data()), size);

	return buffer;
}

std::vector<unsigned char> decryptDatFile(const std::vector<unsigned char> &dat, const std::string &passphrase) {
	using CipherContext = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;

	if (dat.size() < IV_SIZE) {
		throw ".dat is invalid.";
	}

	std::vector<unsigned char> key(KEY_SIZE);
	EVP_BytesToKey(
		EVP_aes_256_cbc(),
		EVP_sha256(),
		nullptr,
		reinterpret_cast<const unsigned char *>(passphrase.c_str()),
		passphrase.length(),
		1,
		key.data(),
		nullptr
	);

	const std::vector<unsigned char> iv(dat.cbegin(), dat.cbegin() + IV_SIZE);
	const std::vector<unsigned char> body(dat.cbegin() + IV_SIZE, dat.cend());

	auto ctx = CipherContext(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
	EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_cbc(), nullptr, key.data(), iv.data());

	std::vector<unsigned char> decrypted(body.size());

	int len, lenadd;
	EVP_DecryptUpdate(ctx.get(), decrypted.data(), &len, body.data(), static_cast<int>(body.size()));
	EVP_DecryptFinal_ex(ctx.get(), decrypted.data() + len, &lenadd);

	decrypted.resize(static_cast<size_t>(len + lenadd));
	return decrypted;
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

void initialize(const std::string &passphrase) {
	g_dat = decryptDatFile(loadDatFile(), passphrase);
	analyzeDat();
}

std::vector<unsigned char> getAsset(uint32_t id) {
	const auto &entry = g_assetMap[id];
	const unsigned char *data = g_dat.data() + entry.offset;
	return std::vector<unsigned char>(data, data + entry.size);
}

} // namespace asset
