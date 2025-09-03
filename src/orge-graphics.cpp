#include <orge.h>

#include "graphics/renderer/renderer.hpp"
#include "graphics/renderpass/renderpass.hpp"
#include "graphics/resource/buffer.hpp"
#include "graphics/resource/image-user.hpp"
#include "graphics/resource/mesh.hpp"
#include "graphics/resource/sampler.hpp"
#include "graphics/window/swapchain.hpp"
#include "orge-private.hpp"

#define DECLARE_UPDATE_DESC_FUNC(n) \
	uint8_t orgeUpdate##n##Descriptor( \
		const char *renderPassId, \
		const char *pipelineId, \
		const char *id, \
		uint32_t set, \
		uint32_t index, \
		uint32_t binding, \
		uint32_t offset \
	)
#define DEFINE_UPDATE_DESC_FUNC(n, m) \
	DECLARE_UPDATE_DESC_FUNC(n) { \
		TRY( \
			graphics::renderpass::getRenderPass(renderPassId) \
				.getPipeline(pipelineId) \
				.update##m##Descriptor(id, set, index, binding, offset) \
		); \
	}
#define TRY_OR(n) \
	bool result = false; \
	CHECK(n); \
	if (!result) graphics::renderer::renderer().reset(); \
	return static_cast<uint8_t>(result);

// ================================================================================================================== //
//     Window                                                                                                         //
// ================================================================================================================== //

uint8_t orgeIsFullscreen(void) {
	return static_cast<uint8_t>(graphics::window::swapchain().isFullscreen());
}

void orgeSetFullscreen(uint8_t toFullscreen) {
	// NOTE: 発生する例外はすべて致命的であり、TRY_DISCARDで強制終了されるので、返戻型はvoid。
	TRY_DISCARD(graphics::window::swapchain().setFullscreen(static_cast<bool>(toFullscreen)));
}

// ================================================================================================================== //
//     Graphics Resources                                                                                             //
// ================================================================================================================== //

uint8_t orgeCreateBuffer(const char *id, uint64_t size, uint8_t isStorage) {
	TRY(graphics::resource::addBuffer(id, size, static_cast<bool>(isStorage)));
}

void orgeDestroyBuffer(const char *id) {
	graphics::resource::destroyBuffer(id);
}

uint8_t orgeUpdateBuffer(const char *id, const uint8_t *data) {
	TRY(graphics::resource::getBuffer(id).update(data));
}

DEFINE_UPDATE_DESC_FUNC(Buffer, Buffer)

uint8_t orgeLoadImage(const char *file) {
	TRY(graphics::resource::addUserImageFromFile(file));
}

void orgeDestroyImage(const char *file) {
	graphics::resource::destroyUserImage(file);
}

DEFINE_UPDATE_DESC_FUNC(Image, UserImage)

uint8_t orgeCreateSampler(const char *id, uint8_t linearMagFilter, uint8_t linearMinFilter, uint8_t repeat) {
	TRY(graphics::resource::addSampler(
		id,
		static_cast<bool>(linearMagFilter),
		static_cast<bool>(linearMinFilter),
		static_cast<bool>(repeat)
	));
}

void orgeDestroySampler(const char *id) {
	graphics::resource::destroySampler(id);
}

DEFINE_UPDATE_DESC_FUNC(Sampler, Sampler)

DECLARE_UPDATE_DESC_FUNC(InputAttachment) {
	TRY(
		const auto &context = graphics::renderer::renderer().getContext();
		graphics::renderpass::getRenderPass(renderPassId)
			.getPipeline(pipelineId)
			.updateInputAttachmentDescriptor(id, set, index, binding, offset, context.currentIndex());
	);
}

uint8_t orgeLoadMesh(const char *id) {
	TRY(graphics::resource::addMesh(id));
}

// ================================================================================================================== //
//     Text Rendering                                                                                                 //
// ================================================================================================================== //

// uint8_t orgeRasterizeCharacters(const char *id, const char *s) {
// 	TRY(g_graphics->rasterizeCharacters(id, s));
// }
// 
// uint8_t orgePutText(
// 	const char *pipelineId,
// 	const char *fontId,
// 	const char *text,
// 	float x,
// 	float y,
// 	float height,
// 	uint32_t horizontal,
// 	uint32_t vertical
// ) {
// 	TRY(g_graphics->putText(
// 		pipelineId,
// 		fontId,
// 		text,
// 		x,
// 		y,
// 		height,
// 		static_cast<OrgeTextLocationHorizontal>(horizontal),
// 		static_cast<OrgeTextLocationVertical>(vertical)
// 	));
// }
// 
// uint8_t orgeDrawTexts(const char *pipelineId) {
// 	TRY(g_graphics->drawTexts(pipelineId));
// }

// ================================================================================================================== //
//     Rendering                                                                                                      //
// ================================================================================================================== //

uint8_t orgeBeginRender(void) {
	TRY_OR(graphics::renderer::renderer().begin());
}

uint8_t orgeEndRender(void) {
	TRY_OR(graphics::renderer::renderer().end());
}

uint8_t orgeBindMesh(const char *meshId) {
	TRY_OR(graphics::renderer::renderer().getContext().bindMesh(meshId));
}

uint8_t orgeBeginRenderPass(const char *renderPassId) {
	TRY_OR(graphics::renderer::renderer().getContext().beginRenderPass(renderPassId));
}

uint8_t orgeEndRenderPass(void) {
	TRY_OR(graphics::renderer::renderer().getContext().endRenderPass());
}

uint8_t orgeNextSubpass(void) {
	TRY_OR(graphics::renderer::renderer().getContext().nextSubpass());
}

uint8_t orgeBindPipeline(const char *pipelineId, uint32_t const *indices) {
	TRY_OR(graphics::renderer::renderer().getContext().bindPipeline(pipelineId, indices));
}

uint8_t orgeDraw(uint32_t instanceCount, uint32_t instanceOffset) {
	TRY_OR(graphics::renderer::renderer().getContext().draw(instanceCount, instanceOffset));
}

uint8_t orgeDrawDirectly(uint32_t vertexCount, uint32_t instanceCount, uint32_t instanceOffset) {
	TRY_OR(graphics::renderer::renderer().getContext().drawDirectly(vertexCount, instanceCount, instanceOffset));
}
