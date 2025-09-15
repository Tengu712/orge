#include "image-attachment.hpp"

#include "../../config/config.hpp"
#include "../../config/enumconvert.hpp"
#include "../../error/error.hpp"
// NOTE: 本来resourceはcoreにのみ依存したいが、attachmentだけは例外的にwindowを知って良いとする。
#include "../window/swapchain.hpp"

#include <memory>
#include <unordered_map>

namespace graphics::resource {

// NOTE: MSVCでunique_ptrを含むunordered_mapのコピーエラーが発生するので、shared_ptrで管理する。
std::unordered_map<std::string, std::vector<Image>> g_attachmentImages;

void destroyAllAttachmentImages() noexcept {
	g_attachmentImages.clear();
}

void initializeAllAttachmentImages() {
	if (!g_attachmentImages.empty()) {
		throw "attachments already initialized.";
	}
	const auto &swapchain = window::swapchain();
	const auto &images = swapchain.getImages();
	const auto &extent = swapchain.getExtent();
	for (const auto &[id, n]: config::config().attachments) {
		std::vector<Image> v;
		v.reserve(images.size());
		for (size_t i = 0; i < images.size(); ++i) {
			const auto format = config::convertFormat(n.format, swapchain.getFormat());
			const auto aspect = config::getImageAspectFromFormat(n.format);
			if (n.format == config::Format::RenderTarget) {
				v.emplace_back(images[i], format, aspect, 4);
			} else {
				const auto usage = config::getImageUsageFromFormat(n.format);
				v.emplace_back(extent.width, extent.height, nullptr, format, usage, aspect, 4);
			}
		}
		g_attachmentImages.emplace(id, std::move(v));
	}
}

const Image &getAttachmentImage(uint32_t index, const std::string &id) {
	const auto &images = error::at(g_attachmentImages, id, "attachments");
	const auto &image = error::at(images, index, "attachments");
	return image;
}

} // namespace graphics::resource
