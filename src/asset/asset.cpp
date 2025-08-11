#include "asset.hpp"

#include <fstream>
#include <memory>
#include <openssl/evp.h>

namespace asset {

constexpr size_t IV_SIZE = 128;

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

std::vector<unsigned char> decryptDatFile(
	const std::vector<unsigned char> &dat,
	const std::vector<unsigned char> &key
) {
	using CipherContext = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;

	if (dat.size() < IV_SIZE) {
		throw ".dat is invalid.";
	}

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

} // namespace asset
