#include "image-storage.hpp"

#include "../../error/error.hpp"

#include <format>
#include <unordered_map>

namespace graphics::resource {

std::unordered_map<std::string, Image> g_storageImages;

void destroyAllStorageImages() noexcept {
	g_storageImages.clear();
}

void addStorageImage(const std::string &id, uint32_t width, uint32_t height, uint32_t format) {
	if (g_storageImages.contains(id)) {
		throw std::format("storage image '{}' already created.", id);
	}

	vk::Format vkFormat;
	uint32_t chCount;
	switch (format) {
	case 0: // RGBA8
		vkFormat = vk::Format::eR8G8B8A8Unorm;
		chCount = 4;
		break;
	case 1: // RG32F
		vkFormat = vk::Format::eR32G32Sfloat;
		chCount = 2;
		break;
	case 2: // R32F
		vkFormat = vk::Format::eR32Sfloat;
		chCount = 1;
		break;
	default:
		throw std::format("unsupported storage image format: {}", format);
	}

	g_storageImages.try_emplace(
		id,
		width,
		height,
		nullptr,
		vkFormat,
		vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled,
		vk::ImageAspectFlagBits::eColor,
		chCount
	);
}

void destroyStorageImage(const std::string &id) noexcept {
	if (g_storageImages.contains(id)) {
		g_storageImages.erase(id);
	}
}

const Image &getStorageImage(const std::string &id) {
	return error::at(g_storageImages, id, "storage images");
}

} // namespace graphics::resource
