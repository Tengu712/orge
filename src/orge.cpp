#include <orge.hpp>

#include "error.hpp"
#include "graphics/graphics.hpp"

#define CHECK_(n) if (auto e = (n); e != Error::None) return static_cast<int>(e);

const char *orgeConvertErrorMessage(int from) {
	return convertErrorMessage(static_cast<Error>(from));
}

int orgeInitialize(const char *windowTitle, int windowInnerWidth, int windowInnerHeight) {
	CHECK_(graphics::initialize(windowTitle, windowInnerWidth, windowInnerHeight));
	return static_cast<int>(Error::None);
}

void orgeTerminate() {
	graphics::terminate();
}
