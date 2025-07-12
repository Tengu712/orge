#include "subpass.hpp"

#include "utils.hpp"

namespace config {

std::vector<SubpassConfig> parseSubpasses(
	const YAML::Node &node,
	const std::unordered_map<std::string, uint32_t> &attachmentMap,
	std::unordered_map<std::string, uint32_t> &subpassMap
) {
	std::vector<SubpassConfig> subpasses;
	for (const auto &n: node["subpasses"]) {
		const auto id = s(n, "id");
		if (!subpassMap.emplace(id, static_cast<uint32_t>(subpassMap.size())).second) {
			throw std::format("config error: subpass id '{}' is duplicated.", id);
		}

		std::vector<vk::AttachmentReference> inputs;
		if (n["inputs"]) {
			for (const auto &m: n["inputs"]) {
				const auto id = s(m, "id");
				const auto layout = s(m, "layout");
				inputs.emplace_back(
					attachmentMap.at(id),
					layout == "depth-stencil-read-only" ? vk::ImageLayout::eDepthStencilReadOnlyOptimal
					: layout == "shader-read-only" ? vk::ImageLayout::eShaderReadOnlyOptimal
					: throw std::format("config error: subpass input layout '{}' is invalid.", layout)
				);
			}
		}

		std::vector<vk::AttachmentReference> outputs;
		for (const auto &m: n["outputs"]) {
			const auto id = cs(m, "config error: the element of outputs is not a string.");
			outputs.emplace_back(attachmentMap.at(id), vk::ImageLayout::eColorAttachmentOptimal);
		}

		std::optional<vk::AttachmentReference> depth;
		if (n["depth"]) {
			const auto id = s(n["depth"], "id");
			const auto readOnly = b(n["depth"], "read-only");
			depth.emplace(
				attachmentMap.at(id),
				readOnly ? vk::ImageLayout::eDepthStencilReadOnlyOptimal : vk::ImageLayout::eDepthStencilAttachmentOptimal
			);
		}

		subpasses.emplace_back(
			SubpassConfig {
				std::move(inputs),
				std::move(outputs),
				std::move(depth),
			}
		);
	}
	return subpasses;
}

} // namespace config
