#include "graphics.hpp"

#include "core.hpp"
#include "window.hpp"

namespace graphics {

Error initialize(const char *title, int width, int height) {
	CHECK(core::initialize());
	CHECK(window::createWindow(title, width, height));
	return Error::None;
}

void terminate() {
	window::terminate();
	core::terminate();
}

} // namespace graphics
