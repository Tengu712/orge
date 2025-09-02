#include "image-attachment.hpp"

#include "../../config/config.hpp"
#include "../../config/enumconvert.hpp"
#include "../../error/error.hpp"
// NOTE: 本来resourceはcoreにのみ依存したいが、attachmentだけは例外的にwindowを知って良いとする。
#include "../window/swapchain.hpp"

namespace graphics::resource {

std::vector<std::unordered_map<std::string, Image>> g_attachmentImages;

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
	for (size_t i = 0; i < images.size(); ++i) {
		std::unordered_map<std::string, Image> m;
		for (const auto &[id, n]: config::config().attachments) {
			const auto format = config::convertFormat(n.format, swapchain.getFormat());
			const auto aspect = config::getImageAspectFromFormat(n.format);
			if (n.format == config::Format::RenderTarget) {
				m.try_emplace(id, images[i], format, aspect, 4);
			} else {
				const auto usage = config::getImageUsageFromFormat(n.format);
				m.try_emplace(id, extent.width, extent.height, nullptr, format, usage, aspect, 4);
			}
		}
		g_attachmentImages.push_back(std::move(m));
	}
}

const Image &getAttachmentImage(uint32_t index, const std::string &id) {
	const auto &images = error::at(g_attachmentImages, index, "attachments");
	const auto &image = error::at(images, id, "attachments");
	return image;
}

} // namespace graphics::resource
