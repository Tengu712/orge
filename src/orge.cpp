#include <orge.h>

#include "config.hpp"
#include "error.hpp"
#include "graphics/graphics.hpp"
#include "graphics/rendering.hpp"
#include "graphics/window.hpp"

#include <sstream>
#include <vulkan/vulkan.hpp>

const char *orgeGetErrorMessage() {
	return error::getMessage();
}

int orgeInitialize(const char *const yaml) {
	TRY(
		graphics::initialize(config::parse(yaml));
	)
}

int orgeInitializeWith(const char *const yamlFilePath) {
	TRY(
		graphics::initialize(config::parseFromFile(yamlFilePath));
	)
}

int orgeDraw(
	uint32_t pipelineCount,
	const char *const *pipelines,
	const char *mesh,
	uint32_t instanceCount,
	uint32_t instanceOffset
) {
	TRY(graphics::rendering::draw(pipelineCount, pipelines, mesh, instanceCount, instanceOffset));
}

void orgeTerminate() {
	graphics::terminate();
}
