#include "framebuffer.hpp"

#include "../../config/config.hpp"
#include "../core/core.hpp"
#include "../resource/image-attachment.hpp"
#include "../window/swapchain.hpp"

namespace graphics::renderpass {

std::vector<vk::UniqueFramebuffer> createFramebuffers(
	const vk::RenderPass &renderPass,
	const std::string &renderPassId
) {
	const auto &swapchain = window::swapchain();

	std::vector<vk::UniqueFramebuffer> framebuffers;
	framebuffers.reserve(swapchain.getImages().size());

	for (size_t i = 0; i < swapchain.getImages().size(); ++i) {
		const auto &rpconfig = config::config().renderPasses.at(renderPassId);
		std::vector<vk::ImageView> as;
		as.reserve(rpconfig.attachments.size());
		for (const auto &n: rpconfig.attachments) {
			as.push_back(resource::getAttachmentImage(static_cast<uint32_t>(i), n).get());
		}

		const auto &extent = swapchain.getExtent();

		const auto ci = vk::FramebufferCreateInfo()
			.setRenderPass(renderPass)
			.setAttachments(as)
			.setWidth(extent.width)
			.setHeight(extent.height)
			.setLayers(1);
		framebuffers.push_back(core::device().createFramebufferUnique(ci));
	}

	return framebuffers;
}

} // namespace graphics::renderpass
