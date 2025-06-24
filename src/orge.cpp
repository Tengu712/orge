#include <orge.h>

#include "error.hpp"
#include "graphics/graphics.hpp"
#include "graphics/window.hpp"

#define CHECK_(n) if (auto e = (n); e != Error::None) return static_cast<int>(e);

const char *orgeConvertErrorMessage(int from) {
	return convertErrorMessage(static_cast<Error>(from));
}

int orgeInitialize(const char *windowTitle, int windowInnerWidth, int windowInnerHeight) {
	CHECK_(graphics::initialize(windowTitle, windowInnerWidth, windowInnerHeight));
	return static_cast<int>(Error::None);
}

int orgePollEvents() {
	return graphics::window::pollEvents();
}

void orgeTerminate() {
	graphics::terminate();
}
