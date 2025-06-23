#include "graphics.hpp"

#include "renderer.hpp"
#include "window.hpp"

namespace graphics {

Error initialize(const char *title, int width, int height) {
	CHECK(renderer::initialize());
	CHECK(window::createWindow(title, width, height));
	return Error::None;
}

void terminate() {
	window::terminate();
	renderer::terminate();
}

} // namespace graphics
