#include <orge.h>

#include "config.hpp"
#include "error.hpp"
#include "graphics/graphics.hpp"

const char *orgeGetErrorMessage() {
	return error::getMessage();
}

int orgeInitialize(const char *yaml) {
	TRY(
		graphics::initialize(config::parse(yaml));
	)
}

int orgeInitializeWith(const char *yamlFilePath) {
	TRY(
		graphics::initialize(config::parseFromFile(yamlFilePath));
	)
}

void orgeTerminate() {
	graphics::terminate();
}
