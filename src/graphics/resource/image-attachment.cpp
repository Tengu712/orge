#include "image-attachment.hpp"

#include "../../config/config.hpp"
#include "../../config/enumconvert.hpp"
#include "../../error/error.hpp"
// NOTE: 本来resourceはcoreにのみ依存したいが、attachmentだけは例外的にwindowを知って良いとする。
#include "../window/swapchain.hpp"

#include <memory>
#include <unordered_map>

namespace graphics::resource {

// NOTE: MSVCだと謎のコンパイルエラーが発生するので、unique_ptrで管理する。
std::vector<std::unordered_map<std::string, std::unique_ptr<Image>>> g_attachmentImages;

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
	g_attachmentImages.resize(images.size());
	for (size_t i = 0; i < images.size(); ++i) {
		for (const auto &[id, n]: config::config().attachments) {
			const auto format = config::convertFormat(n.format, swapchain.getFormat());
			const auto aspect = config::getImageAspectFromFormat(n.format);
			if (n.format == config::Format::RenderTarget) {
				g_attachmentImages[i].emplace(id, std::make_unique<Image>(images[i], format, aspect, 4));
			} else {
				const auto usage = config::getImageUsageFromFormat(n.format);
				g_attachmentImages[i].emplace(id, std::make_unique<Image>(extent.width, extent.height, nullptr, format, usage, aspect, 4));
			}
		}
	}
}

const Image &getAttachmentImage(uint32_t index, const std::string &id) {
	const auto &images = error::at(g_attachmentImages, index, "attachments");
	const auto &image = error::at(images, id, "attachments");
	return *image;
}

} // namespace graphics::resource
