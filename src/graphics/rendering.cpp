#include "rendering.hpp"

#include <vector>

namespace graphics::rendering {

vk::RenderPass g_renderPass;

Error createRenderPass(const Config &config, const vk::Device &device) {
	std::vector<vk::AttachmentDescription> attachments;
	for (const auto &n: config.attachments) {
		attachments.emplace_back(
			vk::AttachmentDescriptionFlags(),
			n.format,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			n.discard ? vk::AttachmentStoreOp::eNone : vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			n.finalLayout
		);
	}

	std::vector<vk::SubpassDescription> subpasses;
	for (const auto &n: config.subpasses) {
		subpasses.emplace_back(
			vk::SubpassDescription()
				.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
				.setInputAttachments(n.inputs)
				.setColorAttachments(n.outputs)
				.setPDepthStencilAttachment(n.depth ? &(*n.depth) : nullptr)
		);
	}

	std::vector<vk::SubpassDependency> subpassDeps;
	for (const auto &n: config.subpassDeps) {
		subpassDeps.emplace_back(
			n.src,
			n.dst,
			vk::PipelineStageFlagBits::eAllCommands,
			vk::PipelineStageFlagBits::eAllCommands,
			vk::AccessFlagBits::eMemoryWrite,
			vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite
		);
	}

	const auto ci = vk::RenderPassCreateInfo()
		.setAttachments(attachments)
		.setSubpasses(subpasses)
		.setDependencies(subpassDeps);
	try {
		g_renderPass = device.createRenderPass(ci);
	} catch (...) {
		return Error::CreateRenderPass;
	}

	return Error::None;
}

Error initialize(const Config &config, const vk::Device &device) {
	CHECK(createRenderPass(config, device));
	return Error::None;
}

void terminate(const vk::Device &device) {
	if (g_renderPass) {
		device.destroyRenderPass(g_renderPass);
		g_renderPass = nullptr;
	}
}

} // namespace graphics::rendering
