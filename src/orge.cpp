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

void orgeTerminate() {
	graphics::terminate();
}
