#include <orge.h>

#include "asset/asset.hpp"
#include "audio/audio.hpp"
#include "config/config.hpp"
#include "error/error.hpp"
#include "graphics/graphics.hpp"
#include "input/input.hpp"

#include <cstdlib>
#include <optional>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

#ifdef __APPLE__
# define MODKEY SDL_KMOD_GUI
#else
# define MODKEY SDL_KMOD_ALT
#endif

#define CHECK(n) \
	try { \
		n; \
		result = true; \
	} catch (const char *e) { \
		error::setMessage(std::string(e)); \
	} catch (const std::string &e) { \
		error::setMessage(e); \
	} catch (const vk::Result &e) { \
		handleVkResult(e); \
	} catch (const vk::SystemError &e) { \
		handleVkResult(static_cast<vk::Result>(e.code().value())); \
	} catch (const std::exception &e) { \
		error::setMessage(e.what()); \
	} catch (...) { \
		error::setMessage("unbound error."); \
    }

#define TRY(n)         bool result = false; CHECK(n); return static_cast<uint8_t>(result);
#define TRY_DISCARD(n) bool result = false; CHECK(n); (void)result;

namespace {

std::optional<graphics::Graphics> g_graphics;
std::optional<audio::Audio> g_audio;

void initialize() {
	if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
		throw "failed to prepare for creating a window.";
	}
	if (!SDL_Vulkan_LoadLibrary(nullptr)) {
		throw "failed to load Vulkan.";
	}
	// TODO: クライアントから受け取る。
	asset::initialize();
	g_graphics.emplace();
	g_audio.emplace();
}

void handleVkResult(const vk::Result &e) {
	switch (e) {
	case vk::Result::eErrorInitializationFailed:
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "fatal error", "Failed to initialize Vulkan. Is Vulkan availabe and latest?", nullptr);
		abort();
	case vk::Result::eErrorLayerNotPresent:
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "fatal error", "Some Vulkan layers required are unavailable. Is Vulkan availabe and latest?", nullptr);
		abort();
	case vk::Result::eErrorExtensionNotPresent:
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "fatal error", "Some Vulkan extensions required are unavailable. Is Vulkan availabe and latest?", nullptr);
		abort();
	case vk::Result::eErrorDeviceLost:
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "fatal error", "Graphics device has been lost.", nullptr);
		abort();
	case vk::Result::eErrorOutOfHostMemory:
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "fatal error", "Host memory limit has been reached.", nullptr);
		abort();
	case vk::Result::eErrorOutOfDeviceMemory:
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "fatal error", "Graphics device memory limit has been reached.", nullptr);
		abort();
	case vk::Result::eErrorInvalidVideoStdParametersKHR:
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "fatal error", "Video Std parameter is invalid.", nullptr);
		abort();
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

// ================================================================================================================== //
//     Lifetime Managiment                                                                                            //
// ================================================================================================================== //

uint8_t orgeInitialize(const char *yaml) {
	TRY(
		config::initializeConfig(yaml);
		initialize();
	)
}

uint8_t orgeInitializeWith(const char *yamlFilePath) {
	TRY(
		config::initializeConfigFromFile(yamlFilePath);
		initialize();
	)
}

void orgeTerminate(void) {
	g_graphics.reset();
	g_audio.reset();
}

uint8_t orgeUpdate(void) {
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
			const auto isFullscreen = g_graphics->isFullscreen();
			// TODO: 例外どうしよう。
			g_graphics->setFullscreen(!isFullscreen);
		}
	}
	input::update();
	// TODO: 例外どうしよう。
	g_audio->update();
	return 1;
}

// ================================================================================================================== //
//     Window                                                                                                         //
// ================================================================================================================== //

uint8_t orgeIsFullscreen(void) {
	return static_cast<uint8_t>(g_graphics->isFullscreen());
}

void orgeSetFullscreen(uint8_t toFullscreen) {
	// NOTE: 発生する例外はすべて致命的であり、TRY_DISCARDで強制終了されるので、返戻型はvoid。
	TRY_DISCARD(g_graphics->setFullscreen(static_cast<bool>(toFullscreen)));
}

// ================================================================================================================== //
//     Graphics Resources                                                                                             //
// ================================================================================================================== //

uint8_t orgeCreateBuffer(const char *id, uint64_t size, uint8_t isStorage) {
	TRY(g_graphics->createBuffer(id, size, static_cast<bool>(isStorage)));
}

void orgeDestroyBuffer(const char *id) {
	g_graphics->destroyBuffer(id);
}

uint8_t orgeUpdateBuffer(const char *id, const uint8_t *data) {
	TRY(g_graphics->updateBuffer(id, static_cast<const void *>(data)));
}

uint8_t orgeUpdateBufferDescriptor(
	const char *bufferId,
	const char *pipelineId,
	uint32_t set,
	uint32_t index,
	uint32_t binding,
	uint32_t offset
) {
	TRY(g_graphics->updateBufferDescriptor(bufferId, pipelineId, set, index, binding, offset));
}

uint8_t orgeCreateImage(const char *id, uint32_t width, uint32_t height, const uint8_t *pixels) {
	TRY(g_graphics->createImage(id, width, height, pixels));
}

uint8_t orgeCreateImageFromFile(const char *id, const char *path) {
	TRY(g_graphics->createImage(id, path));
}

void orgeDestroyImage(const char *id) {
	g_graphics->destroyImage(id);
}

uint8_t orgeUpdateImageDescriptor(
	const char *imageId,
	const char *pipelineId,
	uint32_t set,
	uint32_t index,
	uint32_t binding,
	uint32_t offset
) {
	TRY(g_graphics->updateImageDescriptor(imageId, pipelineId, set, index, binding, offset));
}

uint8_t orgeCreateSampler(const char *id, uint8_t linearMagFilter, uint8_t linearMinFilter, uint8_t repeat) {
	TRY(g_graphics->createSampler(
		id,
		static_cast<bool>(linearMagFilter),
		static_cast<bool>(linearMinFilter),
		static_cast<bool>(repeat)
	));
}

void orgeDestroySampler(const char *id) {
	g_graphics->destroySampler(id);
}

uint8_t orgeUpdateSamplerDescriptor(
	const char *samplerId,
	const char *pipelineId,
	uint32_t set,
	uint32_t index,
	uint32_t binding,
	uint32_t offset
) {
	TRY(g_graphics->updateSamplerDescriptor(samplerId, pipelineId, set, index, binding, offset));
}

uint8_t orgeCreateMesh(
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
//     Text Rendering                                                                                                 //
// ================================================================================================================== //

uint8_t orgeRasterizeCharacters(const char *id, const char *s) {
	TRY(g_graphics->rasterizeCharacters(id, s));
}

uint8_t orgePutText(
	const char *pipelineId,
	const char *fontId,
	const char *text,
	float x,
	float y,
	float height,
	uint32_t horizontal,
	uint32_t vertical
) {
	TRY(g_graphics->putText(
		pipelineId,
		fontId,
		text,
		x,
		y,
		height,
		static_cast<OrgeTextLocationHorizontal>(horizontal),
		static_cast<OrgeTextLocationVertical>(vertical)
	));
}

uint8_t orgeDrawTexts(const char *pipelineId) {
	TRY(g_graphics->drawTexts(pipelineId));
}

// ================================================================================================================== //
//     Rendering                                                                                                      //
// ================================================================================================================== //

#define TRY_OR(n) \
	bool result = false; \
	CHECK(n); \
	if (!result) g_graphics->resetRendering(); \
	return static_cast<uint8_t>(result);

uint8_t orgeBeginRender(void) {
	TRY_OR(g_graphics->beginRender());
}

uint8_t orgeBindDescriptorSets(const char *pipelineId, uint32_t const *indices) {
	TRY_OR(g_graphics->bindDescriptorSets(pipelineId, indices));
}

uint8_t orgeDraw(const char *pipelineId, const char *meshId, uint32_t instanceCount, uint32_t instanceOffset) {
	TRY_OR(g_graphics->draw(pipelineId, meshId, instanceCount, instanceOffset));
}

uint8_t orgeNextSubpass(void) {
	TRY_OR(g_graphics->nextSubpass());
}

uint8_t orgeEndRender(void) {
	TRY_OR(g_graphics->endRender());
}

#undef TRY_OR

// ================================================================================================================== //
//     Input                                                                                                          //
// ================================================================================================================== //

int32_t orgeGetKeyState(uint32_t scancode) {
	return input::getState(static_cast<OrgeScancode>(scancode));
}

// ================================================================================================================== //
//     Audio                                                                                                          //
// ================================================================================================================== //

float orgeGetAudioChannelVolume(uint32_t index) {
	try {
		return g_audio->getVolume(index);
	} catch (...) {
		return -1.0f;
	}
}

uint8_t orgeSetAudioChannelVolume(uint32_t index, float volume) {
	TRY(g_audio->setVolume(index, volume));
}

uint8_t orgeLoadWaveFromFile(const char *id, const char *path, uint32_t startPosition) {
	TRY(g_audio->loadWaveFromFile(id, path, startPosition));
}

void orgeDestroyWave(const char *id) {
	g_audio->destroyWave(id);
}

uint8_t orgePlayWave(const char *id, uint32_t index, uint8_t loop) {
	TRY(g_audio->play(id, index, static_cast<bool>(loop)));
}
