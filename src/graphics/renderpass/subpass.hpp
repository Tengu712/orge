#pragma once

#include "../../config/config.hpp"
#include "../../config/enumconvert.hpp"

#include <vulkan/vulkan.hpp>

namespace graphics::renderpass {

/// レンダーパス作成用の一時オブジェクト
///
/// configに従ってvk::SubpassDescriptionを作成する。
class Subpass {
private:
	std::vector<vk::AttachmentReference> inputs;
	std::vector<vk::AttachmentReference> outputs;
	std::optional<vk::AttachmentReference> depth;

public:
	Subpass(const config::RenderPassConfig &rpconfig, const config::SubpassConfig &spconfig) {
		for (const auto &m: spconfig.inputs) {
			const auto &attachment = config::config().attachments.at(m);
			inputs.emplace_back(rpconfig.attachmentMap.at(m), config::getImageLayoutFromFormat(attachment.format));
		}
		for (const auto &m: spconfig.outputs) {
			outputs.emplace_back(rpconfig.attachmentMap.at(m), vk::ImageLayout::eColorAttachmentOptimal);
		}
		if (spconfig.depth) {
			depth.emplace(
				rpconfig.attachmentMap.at(spconfig.depth->id),
				spconfig.depth->readOnly
					? vk::ImageLayout::eDepthStencilReadOnlyOptimal
					: vk::ImageLayout::eDepthStencilAttachmentOptimal
			);
		}
	}

	vk::SubpassDescription build() const {
		return vk::SubpassDescription()
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setInputAttachments(inputs)
			.setColorAttachments(outputs)
			.setPDepthStencilAttachment(depth ? &depth.value() : nullptr);
	}
};

} // namespace graphics::renderpass
