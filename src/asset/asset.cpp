#include "asset.hpp"

#include <fstream>
#include <memory>
#include <openssl/evp.h>
#include <vector>

namespace asset {

constexpr size_t KEY_SIZE = 32;
constexpr size_t IV_SIZE = 16;

std::vector<unsigned char> g_dat;

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

void initialize(const std::string &passphrase) {
	g_dat = decryptDatFile(loadDatFile(), passphrase);
}

} // namespace asset
