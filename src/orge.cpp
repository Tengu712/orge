#include <orge.h>

#include "config/config.hpp"
#include "error/error.hpp"
#include "graphics/graphics.hpp"
#include "input/input.hpp"

#include <cstdlib>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

#ifdef __APPLE__
# define MODKEY SDL_KMOD_GUI
#else
# define MODKEY SDL_KMOD_ALT
#endif

#define TRY_WITH(n, s, f) \
	try { \
		n; \
		return s; \
	} catch (const char *e) { \
		error::setMessage(std::string(e)); \
		return f; \
	} catch (const std::string &e) { \
		error::setMessage(e); \
		return f; \
	} catch (const vk::Result &e) { \
		handleVkResult(e); \
		return f; \
	} catch (const vk::SystemError &e) { \
		handleVkResult(static_cast<vk::Result>(e.code().value())); \
		return f; \
	} catch (const std::exception &e) { \
		error::setMessage(e.what()); \
		return f; \
	} catch (...) { \
		error::setMessage("unbound error."); \
		return f; \
    }

#define TRY(n) TRY_WITH(n, 1, 0)

namespace {

std::unique_ptr<graphics::Graphics> g_graphics;

void initialize() {
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		throw "failed to prepare for creating a window.";
	}
	if (!SDL_Vulkan_LoadLibrary(nullptr)) {
		throw "failed to load Vulkan.";
	}
	g_graphics = std::make_unique<graphics::Graphics>();
}

void handleVkResult(const vk::Result &e) {
	switch (e) {
	case vk::Result::eErrorInitializationFailed:
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "fatal error", "Failed to initialize Vulkan. Is Vulkan availabe and latest?", nullptr);
		std::exit(1);
	case vk::Result::eErrorLayerNotPresent:
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "fatal error", "Some Vulkan layers required are unavailable. Is Vulkan availabe and latest?", nullptr);
		std::exit(1);
	case vk::Result::eErrorExtensionNotPresent:
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "fatal error", "Some Vulkan extensions required are unavailable. Is Vulkan availabe and latest?", nullptr);
		std::exit(1);
	case vk::Result::eErrorDeviceLost:
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "fatal error", "Graphics device has been lost.", nullptr);
		std::exit(1);
	case vk::Result::eErrorOutOfHostMemory:
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "fatal error", "Host memory limit has been reached.", nullptr);
		std::exit(1);
	case vk::Result::eErrorOutOfDeviceMemory:
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "fatal error", "Graphics device memory limit has been reached.", nullptr);
		std::exit(1);
	case vk::Result::eErrorInvalidVideoStdParametersKHR:
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "fatal error", "Video Std parameter is invalid.", nullptr);
		std::exit(1);
	case vk::Result::eSuboptimalKHR:
	case vk::Result::eErrorOutOfDateKHR:
		error::setMessage("Swapchain is out of date, performing recreation.");
		g_graphics->recreateSwapchain();
		break;
	case vk::Result::eErrorSurfaceLostKHR:
		error::setMessage("Surface is out of date, performing recreation.");
		g_graphics->recreateSurface();
		break;
	default:
		error::setMessage("vulkan error: " + vk::to_string(e) + " (" + std::to_string(static_cast<int64_t>(e)) + ")");
		break;
	}
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

void orgeShowErrorDialog(void) {
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "error", orgeGetErrorMessage(), nullptr);
}

// ================================================================================================================== //
//     Lifetime Managiment                                                                                            //
// ================================================================================================================== //

int orgeInitialize(const char *yaml) {
	TRY(
		config::initializeConfig(yaml);
		initialize();
	)
}

int orgeInitializeWith(const char *yamlFilePath) {
	TRY(
		config::initializeConfigFromFile(yamlFilePath);
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
		if (
			config::config().altReturnToggleFullscreen
				&& event.type == SDL_EVENT_KEY_DOWN
				&& event.key.key == SDLK_RETURN
				&& (event.key.mod & MODKEY)
		) {
			g_graphics->toggleFullscreen();
		}
	}
	input::update();
	return 1;
}

// ================================================================================================================== //
//     Window                                                                                                         //
// ================================================================================================================== //

int orgeIsFullscreen(void) {
	return static_cast<int>(g_graphics->isFullscreen());
}

void orgeSetFullscreen(int toFullscreen) {
	// NOTE: 発生する例外はすべて致命的であり、TRY_WITHで強制終了されるので、返戻型はvoid。
	TRY_WITH(g_graphics->setFullscreen(toFullscreen), , );
}

void orgeToggleFullscreen(void) {
	// NOTE: 発生する例外はすべて致命的であり、TRY_WITHで強制終了されるので、返戻型はvoid。
	TRY_WITH(g_graphics->toggleFullscreen(), , );
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

int orgeBeginRender(void) {
	TRY(g_graphics->beginRender());
}

int orgeBindDescriptorSets(const char *pipelineId, uint32_t const *indices) {
	TRY(g_graphics->bindDescriptorSets(pipelineId, indices));
}

int orgeDraw(const char *pipelineId, const char *meshId, uint32_t instanceCount, uint32_t instanceOffset) {
	TRY(g_graphics->draw(pipelineId, meshId, instanceCount, instanceOffset));
}

int orgeNextSubpass(void) {
	TRY(g_graphics->nextSubpass());
}

int orgeEndRender(void) {
	TRY(g_graphics->endRender());
}

// ================================================================================================================== //
//     Input                                                                                                          //
// ================================================================================================================== //

int32_t orgeGetKeyState(uint32_t scancode) {
	return input::getState(static_cast<OrgeScancode>(scancode));
}
