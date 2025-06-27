#include <orge.h>

#include "config.hpp"
#include "error.hpp"
#include "graphics/graphics.hpp"
#include "graphics/window.hpp"

#define CHECK_(n) if (auto e = (n); e != Error::None) return static_cast<int>(e);

const char *orgeConvertErrorMessage(int from) {
	return convertErrorMessage(static_cast<Error>(from));
}

int initialize(const Config config) {
	CHECK_(graphics::initialize(config));
	return static_cast<int>(Error::None);
}

int orgeInitialize(const char *const yaml) {
	const auto config = parseConfig(yaml);
	if (!config) {
		return static_cast<int>(Error::InvalidConfig);
	}
	return initialize(config.value());
}

int orgeInitializeWith(const char *const yamlFilePath) {
	const auto config = parseConfigFromFile(yamlFilePath);
	if (!config) {
		return static_cast<int>(Error::InvalidConfig);
	}
	return initialize(config.value());
}

int orgePollEvents() {
	return graphics::window::pollEvents();
}

int orgeRender() {
	return static_cast<int>(Error::None);
}

void orgeTerminate() {
	graphics::terminate();
}
