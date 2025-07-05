#include <orge.h>

#include "config.hpp"
#include "error.hpp"
#include "graphics/graphics.hpp"
#include "graphics/rendering.hpp"
#include "graphics/window.hpp"

#include <sstream>
#include <vulkan/vulkan.hpp>

#define TRY(n)                                    \
	try {                                         \
		n;                                        \
		return 1;                                 \
	} catch (const char *e) {                     \
		error::setMessage(std::string(e));        \
		return 0;                                 \
	} catch (const std::string &e) {              \
		error::setMessage(e);                     \
		return 0;                                 \
	} catch (const vk::SystemError &e) {          \
		error::setMessage(std::string(e.what())); \
		return 0;                                 \
	} catch (...) {                               \
		error::setMessage("unbound error.");      \
		return 0;                                 \
    }

const char *orgeGetErrorMessage() {
	return error::getMessage();
}

int orgeInitialize(const char *const yaml) {
	TRY(
		graphics::initialize(parseConfig(yaml));
	)
}

int orgeInitializeWith(const char *const yamlFilePath) {
	TRY(
		graphics::initialize(parseConfigFromFile(yamlFilePath));
	)
}

int orgeCreateMesh(
	const char *id,
	const uint32_t vertexCount,
	const float *vertices,
	const uint32_t indexCount,
	const uint32_t *indices
) {
	TRY(graphics::createMesh(id, vertexCount, vertices, indexCount, indices));
}

int orgePollEvents() {
	return graphics::window::pollEvents();
}

int orgeBeginRender() {
	TRY(graphics::beginRender());
}

int orgeDraw(uint32_t pipelineCount, const char *const *pipelines, const char *mesh, uint32_t instanceCount, uint32_t instanceOffset) {
	TRY(graphics::rendering::draw(pipelineCount, pipelines, mesh, instanceCount, instanceOffset));
}

int orgeEndRender() {
	TRY(graphics::endRender());
}

void orgeTerminate() {
	graphics::terminate();
}
