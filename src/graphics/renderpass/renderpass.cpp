#include "renderpass.hpp"

#include "../../error/error.hpp"
#include "../../config/config.hpp"
#include "../../config/enumconvert.hpp"
#include "../core/core.hpp"
#include "../window/swapchain.hpp"
#include "framebuffer.hpp"
#include "subpass.hpp"

namespace graphics::renderpass {

vk::RenderPass createRenderPass(const std::string &id) {
	const auto &rpconfig = config::config().renderPasses.at(id);
	const auto &swapchain = window::swapchain();

	// アタッチメント
	std::vector<vk::AttachmentDescription> attachments;
	attachments.reserve(rpconfig.attachments.size());
	for (const auto &n: rpconfig.attachments) {
		const auto &m = config::config().attachments.at(n);
		attachments.emplace_back(
			vk::AttachmentDescriptionFlags(),
			config::convertFormat(m.format, swapchain.getFormat()),
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			m.discard
				? vk::AttachmentStoreOp::eNone
				: vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			config::getImageLayoutFromFormat(m.format)
		);
	}

	// サブパス & サブパス依存
	std::vector<Subpass> tempSubpasses;
	std::vector<vk::SubpassDescription> subpasses;
	std::vector<vk::SubpassDependency> dependencies;
	tempSubpasses.reserve(rpconfig.subpasses.size());
	subpasses.reserve(rpconfig.subpasses.size());
	for (const auto &n: rpconfig.subpasses) {
		tempSubpasses.emplace_back(rpconfig, n);
		subpasses.push_back(tempSubpasses.back().build());
		for (const auto &m: n.depends) {
			dependencies.emplace_back(
				rpconfig.subpassMap.at(m),
				rpconfig.subpassMap.at(n.id),
				vk::PipelineStageFlagBits::eAllCommands,
				vk::PipelineStageFlagBits::eAllCommands,
				vk::AccessFlagBits::eMemoryWrite,
				vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite
			);
		}
	}

	// 作成
	const auto ci = vk::RenderPassCreateInfo()
		.setAttachments(attachments)
		.setSubpasses(subpasses)
		.setDependencies(dependencies);
	return core::device().createRenderPass(ci);
}

std::vector<vk::ClearValue> collectClearValues(const std::string &id) {
	const auto &rpconfig = config::config().renderPasses.at(id);
	std::vector<vk::ClearValue> clearValues;
	clearValues.reserve(rpconfig.attachments.size());
	for (const auto &n: rpconfig.attachments) {
		const auto &m = config::config().attachments.at(n).clearValue;
		if (std::holds_alternative<config::ColorClearValue>(m)) {
			clearValues.push_back(static_cast<vk::ClearValue>(vk::ClearColorValue(std::get<config::ColorClearValue>(m))));
		} else {
			clearValues.push_back(static_cast<vk::ClearValue>(vk::ClearDepthStencilValue(std::get<config::DepthClearValue>(m))));
		}
	}
	return clearValues;
}

RenderPass::RenderPass(const std::string &id):
	_id(id),
	_renderPass(createRenderPass(id)),
	_clearValues(collectClearValues(id)),
	_framebuffers(createFramebuffers(_renderPass, id)),
	_pipelines(createPipelines(_renderPass, _id)),
	_trPipelines(createTextRenderingPipelines(_renderPass, _id))
{}

RenderPass::~RenderPass() {
	_pipelines.clear();
	for (const auto &n: _framebuffers) {
		core::device().destroy(n);
	}
	core::device().destroy(_renderPass);
}

void RenderPass::begin(const vk::CommandBuffer &commandBuffer, uint32_t index) const noexcept {
	const auto &extent = window::swapchain().getExtent();
	const auto rbi = vk::RenderPassBeginInfo()
		.setRenderPass(_renderPass)
		.setFramebuffer(_framebuffers[index])
		.setRenderArea(vk::Rect2D({0, 0}, extent))
		.setClearValues(_clearValues);
	commandBuffer.beginRenderPass(rbi, vk::SubpassContents::eInline);
}

void RenderPass::destroyFramebuffersAndPipelines() noexcept {
	for (const auto &n: _framebuffers) {
		core::device().destroy(n);
	}
	_framebuffers.clear();
	_pipelines.clear();
	_trPipelines.clear();
}

void RenderPass::createFramebuffersAndPipelines() {
	_framebuffers = createFramebuffers(_renderPass, _id);
	_pipelines = createPipelines(_renderPass, _id);
	_trPipelines = createTextRenderingPipelines(_renderPass, _id);
}

std::unordered_map<std::string, RenderPass> g_renderPasses;

void initializeRenderPasses() {
	if (!g_renderPasses.empty()) {
		throw "render passes already initialized.";
	}
	g_renderPasses.reserve(config::config().renderPasses.size());
	for (const auto &[id, n]: config::config().renderPasses) {
		g_renderPasses.emplace(id, id);
	}
}

void destroyRenderPasses() noexcept {
	g_renderPasses.clear();
}

const RenderPass &getRenderPass(const std::string &id) {
	return error::at(g_renderPasses, id, "render passes");
}

void destroyAllFramebuffersAndPipelines() noexcept {
	for (auto &[_, n]: g_renderPasses) {
		n.destroyFramebuffersAndPipelines();
	}
}

void createAllFramebuffersAndPipelines() {
	for (auto &[_, n]: g_renderPasses) {
		n.createFramebuffersAndPipelines();
	}
}

} // namespace graphics::renderpass
