#include "image-user.hpp"

#include "../../asset/asset.hpp"
#include "../../config/config.hpp"
#include "../../error/error.hpp"

#include <memory>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace graphics::resource {

std::unordered_map<std::string, Image> g_userImages;

void destroyAllUserImages() noexcept {
	g_userImages.clear();
}

void addUserImageFromFile(const std::string &file) {
	if (g_userImages.contains(file)) {
		throw std::format("image '{}' already created.", file);
	}

	using stbi_ptr = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>;

	const auto assetId = error::at(config::config().assetMap, file, "assets");
	const auto data = asset::getAsset(assetId);

	// NOTE: MSVCの警告を逃れるため。
	const auto dataSize = static_cast<int>(static_cast<uint32_t>(data.size()));

	int width = 0;
	int height = 0;
	int channelCount = 0;
	const auto pixels = stbi_ptr(
		stbi_load_from_memory(data.data(), dataSize, &width, &height, &channelCount, 0),
		stbi_image_free
	);
	if (!pixels) {
		throw std::format("failed to load '{}'.", file);
	}
	if (channelCount != 4) {
		throw std::format("'{}' is not RGBA.", file);
	}

	g_userImages.try_emplace(file,
		width,
		height,
		pixels.get(),
		vk::Format::eR8G8B8A8Srgb,
		vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
		vk::ImageAspectFlagBits::eColor,
		4
	);
}

void destroyUserImage(const std::string &id) noexcept {
	if (g_userImages.contains(id)) {
		g_userImages.erase(id);
	}
}

const Image &getUserImage(const std::string &id) {
	return error::at(g_userImages, id, "images");
}

} // namespace graphics::resource
