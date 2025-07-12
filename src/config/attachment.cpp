#include "attachment.hpp"

#include "../graphics/platform.hpp"
#include "utils.hpp"

namespace config {

std::vector<vk::AttachmentDescription> parseAttachments(
	const YAML::Node &node,
	std::unordered_map<std::string, uint32_t> &attachmentMap
) {
	std::vector<vk::AttachmentDescription> attachments;
	for (const auto &n: node["attachments"]) {
		const auto id = s(n, "id");
		if (!attachmentMap.emplace(id, static_cast<uint32_t>(attachmentMap.size())).second) {
			throw std::format("config error: attachment id '{}' is duplicated.", id);
		}

		const auto format = s(n, "format");
		const auto discard = n["discard"].as<bool>(false);
		const auto finalLayout = s(n, "final-layout");

		attachments.emplace_back(
			vk::AttachmentDescriptionFlags(),
			format == "render-target" ? graphics::platform::getRenderTargetPixelFormat()
			: throw std::format("config error: attachment format '{}' is invalid.", format),
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			discard ? vk::AttachmentStoreOp::eNone : vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			finalLayout == "color-attachment" ? vk::ImageLayout::eColorAttachmentOptimal
			: finalLayout == "depth-stencil-attachment" ? vk::ImageLayout::eDepthStencilAttachmentOptimal
			: finalLayout == "present-src" ? vk::ImageLayout::ePresentSrcKHR
			: throw std::format("config error: attachment final layout '{}' is invalid.", finalLayout)
		);
	}
	return attachments;
}

std::vector<vk::ClearValue> parseClearValues(const YAML::Node &node) {
	std::vector<vk::ClearValue> clearValues;
	for (const auto &n: node["attachments"]) {
		const auto clearValue = n["clear-value"];
		if (clearValue.IsSequence() && clearValue.size() == 4) {
			clearValues.emplace_back(
				static_cast<vk::ClearValue>(
					vk::ClearColorValue(
						cf(clearValue[0], "config error: clear-value is not float."),
						cf(clearValue[1], "config error: clear-value is not float."),
						cf(clearValue[2], "config error: clear-value is not float."),
						cf(clearValue[3], "config error: clear-value is not float.")
					)
				)
			);
		} else if (clearValue.IsScalar()) {
			clearValues.emplace_back(
				static_cast<vk::ClearValue>(
					vk::ClearDepthStencilValue(
						cf(clearValue, "config error: clear-value is not float."),
						0
					)
				)
			);
		} else {
			throw std::format("config error: attachment clear value format is invalid.");
		}
	}
	return clearValues;
}

} // namespace config
