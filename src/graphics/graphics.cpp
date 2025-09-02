#include "graphics.hpp"

#include "core/core.hpp"
#include "renderer/renderer.hpp"
#include "renderpass/renderpass.hpp"
#include "resource/buffer.hpp"
#include "resource/descpool.hpp"
#include "resource/image-attachment.hpp"
#include "resource/image-user.hpp"
#include "resource/mesh.hpp"
#include "resource/sampler.hpp"
#include "window/swapchain.hpp"
#include "utils.hpp"

namespace graphics {

void initialize() {
	core::initializeCore();
	initializeUtils();
	window::initializeSwapchain();
	resource::initializeDescriptorPool();
	resource::initializeAllAttachmentImages();
	renderpass::initializeRenderPasses();
	renderer::initializeRenderer();
}

void terminate() noexcept {
	core::device().waitIdle();
	renderer::destroyRenderer();
	renderpass::destroyRenderPasses();
	resource::destroyAllSamplers();
	resource::destroyAllMeshes();
	resource::destroyAllUserImages();
	resource::destroyAllAttachmentImages();
	resource::destroyDescriptorPool();
	resource::destroyAllBuffers();
	window::destroySwapchain();
	terminateUtils();
	core::destroyCore();
}

} // namespace graphics
