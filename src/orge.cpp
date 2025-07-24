#include <orge.h>

#include "config/config.hpp"
#include "error/error.hpp"
#include "graphics/graphics.hpp"
#include "input/input.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

#ifdef __APPLE__
# define MODKEY SDL_KMOD_GUI
#else
# define MODKEY SDL_KMOD_ALT
#endif

#define TRY(n) \
	try { \
		n; \
		return 1; \
	} catch (const char *e) { \
		error::setMessage(std::string(e)); \
		return 0; \
	} catch (const std::string &e) { \
		error::setMessage(e); \
		return 0; \
	} catch (const vk::Result &e) { \
		error::setMessage("vulkan result: " + std::to_string(static_cast<int64_t>(e))); \
		return 0; \
	} catch (const vk::SystemError &e) { \
		error::setMessage(std::string(e.what())); \
		return 0; \
	} catch (const std::exception &e) { \
		error::setMessage(e.what()); \
		return 0; \
	} catch (...) { \
		error::setMessage("unbound error."); \
		return 0; \
    }

namespace {

std::optional<config::Config> g_config;
std::unique_ptr<graphics::Graphics> g_graphics;

void initialize() {
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		throw "failed to prepare for creating a window.";
	}
	if (!SDL_Vulkan_LoadLibrary(nullptr)) {
		throw "failed to load Vulkan.";
	}
	g_graphics = std::make_unique<graphics::Graphics>(g_config.value());
}

} // namespace

// ================================================================================================================== //
//     Information                                                                                                    //
// ================================================================================================================== //

void orgeShowDialog(uint32_t dtype, const char *title, const char *message) {
	SDL_MessageBoxFlags flags = SDL_MESSAGEBOX_ERROR;
	switch (static_cast<OrgeDialogType>(dtype)) {
	case ORGE_DIALOG_TYPE_WARNING:
		flags = SDL_MESSAGEBOX_WARNING;
		break;
	case ORGE_DIALOG_TYPE_INFORMATION:
		flags = SDL_MESSAGEBOX_INFORMATION;
		break;
	default:
		break;
	}
	SDL_ShowSimpleMessageBox(flags, title, message, nullptr);
}

const char *orgeGetErrorMessage(void) {
	return error::getMessage().c_str();
}

void orgeShowErrorDialog() {
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "error", orgeGetErrorMessage(), nullptr);
}

// ================================================================================================================== //
//     Lifetime Managiment                                                                                            //
// ================================================================================================================== //

int orgeInitialize(const char *yaml) {
	TRY(
		g_config = config::parse(yaml);
		initialize();
	)
}

int orgeInitializeWith(const char *yamlFilePath) {
	TRY(
		g_config = config::parseFromFile(yamlFilePath);
		initialize();
	)
}

void orgeTerminate(void) {
	g_graphics = nullptr;
}

int orgeUpdate(void) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_EVENT_QUIT) {
			return 0;
		}
		if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_RETURN && (event.key.mod & MODKEY)) {
			g_graphics->toggleFullscreen();
		}
	}
	input::update();
	return 1;
}

// ================================================================================================================== //
//     Graphics Resources                                                                                             //
// ================================================================================================================== //

int orgeCreateBuffer(const char *id, uint64_t size, int isStorage) {
	TRY(g_graphics->createBuffer(id, size, isStorage));
}

void orgeDestroyBuffer(const char *id) {
	g_graphics->destroyBuffer(id);
}

int orgeUpdateBuffer(const char *id, const void *data) {
	TRY(g_graphics->updateBuffer(id, data));
}

int orgeUpdateBufferDescriptor(
	const char *bufferId,
	const char *pipelineId,
	uint32_t set,
	uint32_t index,
	uint32_t binding,
	uint32_t offset
) {
	TRY(g_graphics->updateBufferDescriptor(bufferId, pipelineId, set, index, binding, offset));
}

int orgeCreateImage(const char *id, uint32_t width, uint32_t height, const unsigned char *pixels) {
	TRY(g_graphics->createImage(id, width, height, pixels));
}

int orgeCreateImageFromFile(const char *id, const char *path) {
	TRY(g_graphics->createImage(id, path));
}

void orgeDestroyImage(const char *id) {
	g_graphics->destroyImage(id);
}

int orgeUpdateImageDescriptor(
	const char *imageId,
	const char *pipelineId,
	uint32_t set,
	uint32_t index,
	uint32_t binding,
	uint32_t offset
) {
	TRY(g_graphics->updateImageDescriptor(imageId, pipelineId, set, index, binding, offset));
}

int orgeCreateSampler(const char *id, int linearMagFilter, int linearMinFilter, int repeat) {
	TRY(g_graphics->createSampler(id, linearMagFilter, linearMinFilter, repeat));
}

void orgeDestroySampler(const char *id) {
	g_graphics->destroySampler(id);
}

int orgeUpdateSamplerDescriptor(
	const char *samplerId,
	const char *pipelineId,
	uint32_t set,
	uint32_t index,
	uint32_t binding,
	uint32_t offset
) {
	TRY(g_graphics->updateSamplerDescriptor(samplerId, pipelineId, set, index, binding, offset));
}

int orgeCreateMesh(
	const char *id,
	const uint32_t vertexCount,
	const float *vertices,
	const uint32_t indexCount,
	const uint32_t *indices
) {
	TRY(g_graphics->createMesh(id, vertexCount, vertices, indexCount, indices));
}

void orgeDestroyMesh(const char *id) {
	g_graphics->destroyMesh(id);
}

// ================================================================================================================== //
//     Rendering                                                                                                      //
// ================================================================================================================== //

int orgeBeginRender() {
	TRY(g_graphics->beginRender(g_config.value()));
}

int orgeBindDescriptorSets(const char *pipelineId, uint32_t const *indices) {
	TRY(g_graphics->bindDescriptorSets(pipelineId, indices));
}

int orgeDraw(const char *pipelineId, const char *meshId, uint32_t instanceCount, uint32_t instanceOffset) {
	TRY(g_graphics->draw(pipelineId, meshId, instanceCount, instanceOffset));
}

int orgeNextSubpass() {
	TRY(g_graphics->nextSubpass());
}

int orgeEndRender() {
	TRY(g_graphics->endRender());
}

// ================================================================================================================== //
//     Input                                                                                                          //
// ================================================================================================================== //

int32_t orgeGetKeyState(uint32_t scancode) {
	return input::getState(static_cast<OrgeScancode>(scancode));
}
